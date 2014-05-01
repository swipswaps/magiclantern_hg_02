

#include <dryos.h>
#include <bmp.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <console.h>
#include <module.h>

#include "sound.h"
#include "../modules/trace/trace.h"

/*
 * Define this to 1 to trace sound API calls
 */
#define SOUND_DEBUG_TRACE 0

static struct sound_dev sound_device;

extern int StartASIFDMAADC(void *, uint32_t, void *, uint32_t, void (*)());
extern int StartASIFDMADAC(void *, uint32_t, void *, uint32_t, void (*)(), uint32_t);
extern int SetNextASIFADCBuffer(void *, uint32_t);
extern int SetNextASIFDACBuffer(void *, uint32_t);
extern int StopASIFDMAADC(void *, void *);
extern int StopASIFDMADAC(void *, void *);
extern void SetASIFMode(int rate, signed int bits, int channels, int output);

uint32_t sound_trace_ctx = TRACE_ERROR;
static struct sound_ctx *sound_settings_ctx = NULL;


static struct sound_mixer default_mixer_options =
{
    .windcut_mode = SOUND_WINDCUT_OFF,
    .source_line = SOUND_SOURCE_INT_MIC,
    .destination_line = SOUND_DESTINATION_SPK,
    .loop_mode = SOUND_LOOP_DISABLED,
    .headphone_agc = SOUND_AGC_DISABLED,
    .headphone_gain = 100,
    .mic_power = SOUND_POWER_ENABLED,
    .mic_agc = SOUND_AGC_DISABLED,
    .mic_gain = 100,
    .speaker_gain = 100,
    .out_gain = 100
};

static struct sound_ops default_sound_ops = 
{
    .apply = &sound_op_apply,
    .lock = &sound_op_lock,
    .unlock = &sound_op_unlock,
    .start = &sound_op_start,
    .enqueue = &sound_op_enqueue,
    .flush = &sound_op_flush,
    .pause = &sound_op_pause,
    .stop = &sound_op_stop
};


/* called when stopping ASIF transfers */
static void sound_asif_stop_cbr(uint32_t priv)
{
    struct sound_ctx *ctx = sound_device.current_ctx;
    struct sound_dev *dev = ctx->device;
    uint32_t msg_count = 0;

    /* we have two buffers currently being played and the ones in the queue */
    msg_queue_count(ctx->buffer_queue, &msg_count);
    
    /* requeue the buffers being played */
    msg_queue_post(ctx->buffer_queue, (uint32_t)dev->current_buffer);
    msg_queue_post(ctx->buffer_queue, (uint32_t)dev->next_buffer);
    
    dev->current_buffer = NULL;
    dev->next_buffer = NULL;
    
    /* pop and requeue the buffers that were at queue front */
    for(uint32_t pos = 0; pos < msg_count; pos++)
    {
        struct sound_buffer *buffer = NULL;
        if(msg_queue_receive(ctx->buffer_queue, &buffer, 50))
        {
            break;
        }
        msg_queue_post(ctx->buffer_queue, (uint32_t)buffer);
    }

    /* now we have the buffers that were played at frontmost position */
    ctx->device->state = SOUND_STATE_CLEANUP;
}

/* called whenever a audio black has been trasferred */
static void sound_asif_cbr(uint32_t priv)
{
    struct sound_ctx *ctx = sound_device.current_ctx;
    struct sound_dev *dev = ctx->device;
    
    trace_write(sound_trace_ctx, "sound_asif_cbr: entered (0x%08X, %d)", priv, dev->state);

    switch(dev->state)
    {
        /* these are unexpected */
        case SOUND_STATE_IDLE:
        case SOUND_STATE_CLEANUP:
            console_printf("state: %d - UNEXPECTED CALL", dev->state);
            break;

        case SOUND_STATE_STARTED:
        case SOUND_STATE_STOPPING:
        case SOUND_STATE_RUNNING:
        {
            enum sound_flow flow = SOUND_FLOW_CONTINUE;
            struct sound_buffer *next_buffer = NULL;
            uint32_t msg_count = 0;

            msg_queue_count(ctx->buffer_queue, &msg_count);     
            trace_write(sound_trace_ctx, "sound_asif_cbr: have %d msgs", msg_count);


            /* if there is no new buffer to fill/play, requeue the current one */
            if((msg_count == 0) || (msg_queue_receive(ctx->buffer_queue, &next_buffer, 50)))
            {
                /* call the buffer cbr for notifying about requeueing */
                next_buffer = dev->current_buffer;
                flow = sound_buf_requeued(next_buffer);
                trace_write(sound_trace_ctx, "sound_asif_cbr: requeue");
            }
            else
            {
                /* call the buffer cbr */
                flow = sound_buf_processed(dev->current_buffer);
                trace_write(sound_trace_ctx, "sound_asif_cbr: processed");
            }

            dev->current_buffer = dev->next_buffer;
            dev->next_buffer = next_buffer;
            
            if(flow == SOUND_FLOW_STOP && dev->state != SOUND_STATE_STOPPING)
            {
                trace_write(sound_trace_ctx, "sound_asif_cbr: sound_stop_asif");
                sound_stop_asif(ctx);
            }
            else
            {
                trace_write(sound_trace_ctx, "sound_asif_cbr: enqueue next buffer");
                
                /* depending on operation mode, call the right queue function */
                if(dev->current_ctx->mode == SOUND_MODE_PLAYBACK)
                {
                    SetNextASIFDACBuffer(next_buffer->data, next_buffer->size);
                }
                else
                {
                    SetNextASIFADCBuffer(next_buffer->data, next_buffer->size);
                }
            }
            break;
        }
    }
}

/* helper to call cleanup function of a buffer */
static enum sound_flow sound_buf_cleanup(struct sound_buffer *buf)
{
    enum sound_flow ret = SOUND_FLOW_CONTINUE;
    
    if(buf && buf->cleanup)
    {
        ret = buf->cleanup(buf);
    }
    
    return ret;
}

/* helper to call requeued function of a buffer */
static enum sound_flow sound_buf_requeued(struct sound_buffer *buf)
{
    enum sound_flow ret = SOUND_FLOW_CONTINUE;
    
    trace_write(sound_trace_ctx, "sound_buf_requeued: check (0x%08X, 0x%08X)", buf, buf->requeued);
    if(buf && buf->requeued)
    {
        trace_write(sound_trace_ctx, "sound_buf_requeued: calling");
        ret = buf->requeued(buf);
    }
    
    return ret;
}

/* helper to call processed function of a buffer */
static enum sound_flow sound_buf_processed(struct sound_buffer *buf)
{
    enum sound_flow ret = SOUND_FLOW_CONTINUE;
    
    trace_write(sound_trace_ctx, "sound_buf_processed: check (0x%08X, 0x%08X)", buf, buf->processed);
    if(buf && buf->processed)
    {
        trace_write(sound_trace_ctx, "sound_buf_processed: calling");
        ret = buf->processed(buf);
    }
    
    return ret;
}

/* check given context if it is the current sound device context */
static enum sound_result sound_check_ctx (struct sound_ctx *ctx)
{
    if(!ctx || !ctx->device)
    {
        return SOUND_RESULT_ERROR;
    }

    if(ctx->device->current_ctx != ctx)
    {
        return SOUND_RESULT_ERROR;
    }

    return SOUND_RESULT_OK;
}


enum sound_state sound_get_state (struct sound_ctx *ctx)
{
    /* our context isnt the one that uses the device */
    if(sound_check_ctx(ctx) != SOUND_RESULT_OK)
    {
        return SOUND_STATE_IDLE;
    }
    
    return ctx->device->state;
}


static void sound_set_mixer(struct sound_ctx *ctx)
{
    struct sound_mixer mixer = ctx->mixer;
    struct sound_mixer prev_mixer = ctx->device->mixer_current;
    
    /* plan: 
        get sound_ctx.mixer and place in sound_dev.mixer_current.
        replace all default settings of sound_ctx with sound_dev.mixer_defaults values.
    */
    sound_mixer_merge(&ctx->device->mixer_current, &ctx->device->mixer_defaults, &mixer);

    ctx->device->codec_ops.apply_mixer(&prev_mixer, &ctx->device->mixer_current);
}

/* set up audio IC for current audio mode */
static void sound_set_codec(struct sound_ctx *ctx)
{
    ctx->device->codec_ops.set_rate(ctx->format.rate);
}

/* set up all ASIF related bits. this is exclusive for the current sound ctx (either play or rec) */
static void sound_set_asif(struct sound_ctx *ctx)
{
    SetASIFMode(ctx->format.rate, (ctx->format.sampletype == SOUND_SAMPLETYPE_SINT16)?16:8, ctx->format.channels, (ctx->mode == SOUND_MODE_PLAYBACK));
}

static void sound_try_start(struct sound_ctx *ctx)
{
    trace_write(sound_trace_ctx, "sound_try_start: enter");
    
    /* audio wasnt started yet, but now we have enough data to start */
    if(ctx->device->state == SOUND_STATE_STARTED)
    {
        uint32_t msg_count = 0;
        
        msg_queue_count(ctx->buffer_queue, &msg_count);
        
        if(msg_count >= 2)
        {
            struct sound_buffer *buffer_1;
            struct sound_buffer *buffer_2;

            /* get next buffers */
            msg_queue_receive(ctx->buffer_queue, &buffer_1, 0);
            msg_queue_receive(ctx->buffer_queue, &buffer_2, 0);

            /* update device state */
            ctx->device->current_buffer = buffer_1;
            ctx->device->next_buffer = buffer_2;
            ctx->device->state = SOUND_STATE_RUNNING;

            /* start operation */
            if(ctx->mode == SOUND_MODE_PLAYBACK)
            {  
                trace_write(sound_trace_ctx, "sound_try_start: StartASIFDMADAC (%d)", ctx->device->state);
                StartASIFDMADAC(buffer_1->data, buffer_1->size, buffer_2->data, buffer_2->size, sound_asif_cbr, (uint32_t)ctx);
            }
            else
            {
                trace_write(sound_trace_ctx, "sound_try_start: StartASIFDMAADC (%d)", ctx->device->state);
                StartASIFDMAADC(buffer_1->data, buffer_1->size, buffer_2->data, buffer_2->size, sound_asif_cbr);
            }
        }
    }
}

static void sound_stop_asif(struct sound_ctx *ctx)
{
    if(ctx->mode == SOUND_MODE_PLAYBACK)
    {
        trace_write(sound_trace_ctx, "sound_stop_asif: StopASIFDMADAC");
        StopASIFDMADAC(&sound_asif_stop_cbr, ctx);
    }
    else
    {
        trace_write(sound_trace_ctx, "sound_stop_asif: StopASIFDMAADC");
        StopASIFDMAADC(&sound_asif_stop_cbr, ctx);
    }
}

static enum sound_result sound_op_lock (struct sound_ctx *ctx, enum sound_lock type)
{
    if(sound_trace_ctx == TRACE_ERROR)
    {
        #if SOUND_DEBUG_TRACE
        sound_trace_ctx = trace_start("snd_test", "B:/snd_test.txt");
        #endif
        trace_set_flushrate(sound_trace_ctx, 1000);
        trace_format(sound_trace_ctx, TRACE_FMT_TIME_REL | TRACE_FMT_COMMENT, ' ');
    }
    
    if(!ctx || !ctx->device)
    {
        return SOUND_RESULT_ERROR;
    }

    /* ToDo: add mutex code here */
    if(ctx->device->lock_type != SOUND_LOCK_UNLOCKED)
    {
        /* take over device if the lock level requested is higher */
        if(ctx->device->lock_type >= type)
        {
            return SOUND_RESULT_ERROR;
        }
        
        ctx->previous_state = ctx->device->lock_type;
        ctx->previous_lock = ctx->device->lock_type;
        ctx->previous_ctx = ctx->device->current_ctx;
        
        /* stop previous playback */
        ctx->previous_ctx->ops.pause(ctx->previous_ctx);
    }
    else
    {
        ctx->previous_ctx = NULL;
    }

    ctx->device->lock_type = type;
    ctx->device->current_ctx = ctx;

    return SOUND_RESULT_OK;
}

static enum sound_result sound_op_unlock (struct sound_ctx *ctx)
{
    if(sound_check_ctx(ctx) != SOUND_RESULT_OK)
    {
        return SOUND_RESULT_ERROR;
    }

    /* did caller forget to clean up? */
    if(ctx->device->state != SOUND_STATE_IDLE)
    {
        ctx->ops.stop(ctx);
    }
    
    struct sound_ctx *new_ctx = NULL;
    enum sound_lock new_lock_type = SOUND_LOCK_UNLOCKED;

    /* restore previous playback context and its state */
    if(ctx->previous_ctx)
    {
        new_ctx = ctx->previous_ctx;
        new_lock_type = ctx->previous_lock;
        
        ctx->previous_ctx = NULL;
    }

    ctx->device->current_ctx = new_ctx;
    ctx->device->lock_type = new_lock_type;
    ctx->device->mixer_current = default_mixer_options;
    
    /* some previously interrupted context gets its control back now */
    if(new_ctx && new_ctx->paused)
    {
        new_ctx->ops.apply(new_ctx);
        new_ctx->ops.start(new_ctx);
    }
    
    return SOUND_RESULT_OK;
}

static enum sound_result sound_op_apply (struct sound_ctx *ctx)
{
    if(!ctx || !ctx->device)
    {
        return SOUND_RESULT_ERROR;
    }
    
    sound_set_mixer(ctx);

    return SOUND_RESULT_OK;
}

static enum sound_result sound_op_start (struct sound_ctx *ctx)
{
    if(sound_check_ctx(ctx) != SOUND_RESULT_OK)
    {
        return SOUND_RESULT_ERROR;
    }

    if(ctx->device->state != SOUND_STATE_IDLE)
    {
        return SOUND_RESULT_ERROR;
    }
    
    /* first setup ASIF DMA stuff */
    sound_set_asif(ctx);
    
    /* make sure audio IC is powered on */
    ctx->device->codec_ops.poweron();
    
    /* then set audio codec's channels, where to record from etc */
    sound_set_codec(ctx);
    sound_set_mixer(ctx);
    
    ctx->device->current_buffer = NULL;
    ctx->device->next_buffer = NULL;
    ctx->device->state = SOUND_STATE_STARTED;
    ctx->paused = 0;
    
    /* maybe there are already buffers queued? try to start */
    sound_try_start(ctx);

    return SOUND_RESULT_OK;
}

static enum sound_result sound_op_enqueue (struct sound_ctx *ctx, struct sound_buffer *buffer)
{
    trace_write(sound_trace_ctx, "sound_op_enqueue: enter");

    if(sound_check_ctx(ctx) != SOUND_RESULT_OK)
    {
        /* if the context was paused, then its no failure to call enqueue */
        if(ctx->paused)
        {
            return SOUND_RESULT_TRYAGAIN;
        }
        
        trace_write(sound_trace_ctx, "sound_op_enqueue: ctx fail");
        return SOUND_RESULT_ERROR;
    }

    uint32_t msg_count = 0;

    msg_queue_count(ctx->buffer_queue, &msg_count);
    trace_write(sound_trace_ctx, "sound_op_enqueue: have %d msgs", msg_count);
    
    /* if just started, we are allowed to queue one buffer more */
    if(msg_count < ctx->device->max_queue_size || ctx->device->state == SOUND_STATE_STARTED)
    {
        buffer->sequence = ctx->buffers_played;
        ctx->buffers_played++;
        
        msg_queue_post(ctx->buffer_queue, (uint32_t)buffer);

        sound_try_start(ctx);

        trace_write(sound_trace_ctx, "sound_op_enqueue: added");
        return SOUND_RESULT_OK;
    }

    trace_write(sound_trace_ctx, "sound_op_enqueue: too many messages");
    return SOUND_RESULT_TRYAGAIN;
}

static enum sound_result sound_op_flush (struct sound_ctx *ctx)
{
    //console_printf("sound_op_flush()\n");
    if(sound_check_ctx(ctx) != SOUND_RESULT_OK)
    {
        return SOUND_RESULT_ERROR;
    }

    while(1)
    {
        struct sound_buffer *buffer = NULL;
        
        if(msg_queue_receive(ctx->buffer_queue, &buffer, 20))
        {
            break;
        }
        
        sound_buf_cleanup(buffer);
    }

    return SOUND_RESULT_OK;
}

static enum sound_result sound_op_pause (struct sound_ctx *ctx)
{
    //console_printf("sound_op_pause()\n");
    if(sound_check_ctx(ctx) != SOUND_RESULT_OK)
    {
        return SOUND_RESULT_ERROR;
    }

    while(ctx->device->state != SOUND_STATE_IDLE)
    {
        switch(ctx->device->state)
        {
            case SOUND_STATE_IDLE:
                break;

            case SOUND_STATE_STARTED:
                /* playback is starting, but not enough buffers queued yet. so simply jump to cleanup */
                /* ToDo: mutex? */
                ctx->device->state = SOUND_STATE_CLEANUP;
                break;

            case SOUND_STATE_RUNNING:
                ctx->device->state = SOUND_STATE_STOPPING;
                sound_stop_asif(ctx);
                break;

            case SOUND_STATE_STOPPING:
                /* nothing to do, wait for asif cbr to enter cleanup */
                msleep(20);
                break;

            case SOUND_STATE_CLEANUP:
                /* ctx queue cleanup is done in stop function, here we only flush buffers being in ASIF */
                ctx->device->state = SOUND_STATE_IDLE;
                break;
        }
    }
    
    ctx->paused = 1;

    return SOUND_RESULT_OK;
}

static enum sound_result sound_op_stop (struct sound_ctx *ctx)
{
    if(sound_check_ctx(ctx) != SOUND_RESULT_OK)
    {
        return SOUND_RESULT_ERROR;
    }
    
    ctx->ops.pause(ctx);
    ctx->ops.flush(ctx);
    ctx->paused = 0;
    
    /* its okay to shut down audio now */
    ctx->device->codec_ops.poweroff();
    
    return SOUND_RESULT_OK;
}

/* allocate a audio buffer */
struct sound_buffer *sound_alloc_buffer()
{
    struct sound_buffer *buf = malloc(sizeof(struct sound_buffer));
    
    memset(buf, 0x00, sizeof(struct sound_buffer));
    
    return buf;
}

/* allocate a context for the default device */
struct sound_ctx *sound_alloc()
{
    struct sound_ctx *ctx = malloc(sizeof(struct sound_ctx));

    if(!ctx)
    {
        return NULL;
    }

    /* use global device, maybe we will add others (e.g. wave file?) */
    ctx->device = &sound_device;
    ctx->device_lost_cbr = NULL;
    ctx->previous_ctx = NULL;
    ctx->buffer_queue = msg_queue_create("sound_queue", sound_device.max_queue_size + 5);
    
    ctx->buffers_played = 0;
    ctx->paused = 0;
    ctx->ops = default_sound_ops;
    
    /* set ctx operations */

    /* default is playback with 48kHz, 16bit stereo */
    ctx->mode = SOUND_MODE_PLAYBACK;
    ctx->format.rate = 48000;
    ctx->format.channels = 2;
    ctx->format.sampletype = SOUND_SAMPLETYPE_SINT16;

    /* mixer settings to default */
    ctx->mixer = default_mixer_options;


    return ctx;
}

/* free a previously allocated context */
void sound_free(struct sound_ctx *ctx)
{
    ctx->ops.stop(ctx);
    
    /* ToDo: delete message queue */
    free(ctx);

}

void sound_free_buffer(struct sound_buffer *buffer)
{
    free(buffer);
}

void sound_mixer_merge(struct sound_mixer *merged, struct sound_mixer *base, struct sound_mixer *settings)
{
    *merged = *settings;
    
    if(merged->windcut_mode == SOUND_WINDCUT_DEFAULT)
    {
        merged->windcut_mode = base->windcut_mode;
    }
    if(merged->source_line == SOUND_SOURCE_DEFAULT)
    {
        merged->source_line = base->source_line;
    }
    if(merged->destination_line == SOUND_DESTINATION_DEFAULT)
    {
        merged->destination_line = base->destination_line;
    }
    if(merged->headphone_agc == SOUND_AGC_DEFAULT)
    {
        merged->headphone_agc = base->headphone_agc;
    }
    if(merged->mic_agc == SOUND_AGC_DEFAULT)
    {
        merged->mic_agc = base->mic_agc;
    }
    if(merged->mic_power == SOUND_POWER_DEFAULT)
    {
        merged->mic_power = base->mic_power;
    }
    if(merged->loop_mode == SOUND_LOOP_DEFAULT)
    {
        merged->loop_mode = base->loop_mode;
    }
    if(merged->headphone_gain == SOUND_GAIN_DEFAULT)
    {
        merged->headphone_gain = base->headphone_gain;
    }
    if(merged->mic_gain == SOUND_GAIN_DEFAULT)
    {
        merged->mic_gain = base->mic_gain;
    }
    if(merged->speaker_gain == SOUND_GAIN_DEFAULT)
    {
        merged->speaker_gain = base->speaker_gain;
    }
    if(merged->out_gain == SOUND_GAIN_DEFAULT)
    {
        merged->out_gain = base->out_gain;
    }
}


void sound_defaults_update()
{
    sound_device.mixer_defaults = default_mixer_options;
    
    if(sound_device.current_ctx)
    {
        struct sound_mixer current = sound_device.current_ctx->mixer;
        struct sound_mixer merged;
        
        sound_mixer_merge(&merged, &sound_device.mixer_defaults, &current);
        
        sound_device.mixer_current = merged;
    }
    
    /* apply new default settings */
    sound_settings_ctx->mixer = sound_device.mixer_current;
    sound_settings_ctx->ops.apply(sound_settings_ctx);
}


static MENU_UPDATE_FUNC(sound_menu_spk_vol_update)
{
    MENU_SET_VALUE("%d %%", default_mixer_options.speaker_gain);
}

static MENU_UPDATE_FUNC(sound_menu_hdp_vol_update)
{
    MENU_SET_VALUE("%d %%", default_mixer_options.headphone_gain);
}

static MENU_UPDATE_FUNC(sound_in_line_update)
{
    enum sound_source line = default_mixer_options.source_line;
    const char *name = sound_device.codec_ops.get_source_name(line);
    
    MENU_SET_VALUE("%s (#%d)", name, line);
}

static MENU_UPDATE_FUNC(sound_out_line_update)
{
    enum sound_destination line = default_mixer_options.destination_line;
    const char *name = sound_device.codec_ops.get_destination_name(line);
    
    MENU_SET_VALUE("%s (#%d)", name, line);
}

static MENU_UPDATE_FUNC(sound_loop_update)
{
    MENU_SET_VALUE("%s", default_mixer_options.loop_mode?"ON":"OFF");
}

static MENU_SELECT_FUNC(sound_menu_spk_vol_select)
{
    default_mixer_options.speaker_gain = COERCE(default_mixer_options.speaker_gain + delta, 0, 100);
    sound_defaults_update();
}

static MENU_SELECT_FUNC(sound_menu_hdp_vol_select)
{
    default_mixer_options.headphone_gain = COERCE(default_mixer_options.headphone_gain + delta, 0, 100);
    sound_defaults_update();
}

static MENU_SELECT_FUNC(sound_in_line_select)
{
    default_mixer_options.source_line = (enum sound_source)COERCE(default_mixer_options.source_line + delta, 0, SOUND_DESTINATION_EXTENDED_6);
    sound_defaults_update();
}

static MENU_SELECT_FUNC(sound_out_line_select)
{
    default_mixer_options.destination_line = (enum sound_destination)COERCE(default_mixer_options.destination_line + delta, 0, SOUND_DESTINATION_EXTENDED_6);
    sound_defaults_update();
}

static MENU_SELECT_FUNC(sound_loop_select)
{
    default_mixer_options.loop_mode = !default_mixer_options.loop_mode;
    sound_defaults_update();
}

static struct menu_entry sound_menu[] =
{
    {
        .name = "Sound defaults",
        .help = "",
        .submenu_width = 710,
        .children = (struct menu_entry[])
        {
            {
                .name = "Speaker volume",
                .update = &sound_menu_spk_vol_update,
                .select = &sound_menu_spk_vol_select,
                .help = "Audio output volume (0-100).",
            },
            {
                .name = "Headphone volume",
                .update = &sound_menu_hdp_vol_update,
                .select = &sound_menu_hdp_vol_select,
                .help = "Audio output volume (0-100).",
            },
            {
                .name = "Output line",
                .update = &sound_out_line_update,
                .select = &sound_out_line_select,
                .max = SOUND_DESTINATION_EXTENDED_6,
                .help = "Audio output line (0-3).",
            },
            {
                .name = "Input line",
                .update = &sound_in_line_update,
                .select = &sound_in_line_select,
                .max = SOUND_SOURCE_EXTENDED_6,
                .help = "Audio input line (0-3).",
            },
            {
                .name = "Loopback",
                .update = &sound_loop_update,
                .select = &sound_loop_select,
                .max = 1,
                .help = "Enable loopback (no effect)",
            },
            MENU_EOL,
        },
    },
};


void sound_init()
{
    sound_device.current_ctx = NULL;
    sound_device.current_buffer = NULL;
    sound_device.next_buffer = NULL;
    sound_device.state = SOUND_STATE_IDLE;
    sound_device.lock_type = SOUND_LOCK_UNLOCKED;
    sound_device.max_queue_size = 5;
    sound_device.mixer_defaults = default_mixer_options;
    sound_device.mixer_current = default_mixer_options;

    /* depending on which camera we have, the correct codec should be linked */
    extern void codec_init(struct codec_ops *ops);
    codec_init(&sound_device.codec_ops);
    
    sound_settings_ctx = sound_alloc();
    menu_add("Audio", sound_menu, COUNT(sound_menu) );
}


INIT_FUNC("sound.init", sound_init);
