#include "GstDisplay.h"
#include <QDebug>





//-------------------------------------------------------------------------------------------------------------
static void ChangeStateHandler (GstElement* pipeline, GstState* stateNewPtr) {

    GstState stateOld;
    GstState stateNew;

    gst_element_get_state(pipeline, &stateOld, NULL,1*GST_SECOND);
    stateNew = *stateNewPtr;

    delete stateNewPtr;

    if(stateOld != stateNew){
        GstStateChangeReturn ret = gst_element_set_state (pipeline, stateNew);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            qWarning() << "Error: cannot change state!";
        }
    }
}




//-------------------------------------------------------------------------------------------------
GstDisplay::GstDisplay(QObject *parent) : QObject(parent) {

    data.pipeline = NULL;

    data.appsrc = NULL;
    data.queue = NULL;
    data.videoconvert = NULL;
    data.videosink = NULL;

    data.pOwner = this;
}



//-------------------------------------------------------------------------------------------------
GstDisplay::~GstDisplay() {

    DisposePipeline();
}



//--------------------------------------------------------------------------
void GstDisplay::InstantiatePipeline(QSize size) {

    if(data.pipeline == NULL) {

        data.pipeline = gst_pipeline_new ("pipeline");

        data.appsrc = gst_element_factory_make("appsrc", NULL);
        data.queue = gst_element_factory_make("queue", NULL);
        data.videoconvert = gst_element_factory_make("videoconvert", NULL);
        data.videosink = gst_element_factory_make("autovideosink", NULL);

        if (!data.pipeline || !data.appsrc || !data.queue || !data.videoconvert || !data.videosink) {
            qWarning() << "Not all elements could be created!";
            DisposePipeline();
            return;
        }

        gst_bin_add_many (GST_BIN(data.pipeline), data.appsrc, data.queue, data.videoconvert, data.videosink, (char*)NULL);

        // Link the pipeline
        if (!gst_element_link_many (data.appsrc, data.queue, data.videoconvert, data.videosink, (char*)NULL)) {
            qWarning() << "Error: not all elements could be linked!";
            DisposePipeline();
            return;
        }

        SetupAppsrc(size);
    }
}



//--------------------------------------------------------------------------
void GstDisplay::DisposePipeline() {

    if(data.pipeline != NULL) {

        GstStop(true);

        gst_object_unref (data.pipeline);

        data.pipeline = NULL;

        data.appsrc = NULL;
        data.queue = NULL;
        data.videoconvert = NULL;
        data.videosink = NULL;

        data.pOwner = NULL;
    }
}





//--------------------------------------------------------------------------
void GstDisplay::ChangeStateAsync(GstState stateNew) {

    GstState* stateNewPtr;
    stateNewPtr = new GstState(stateNew);

    if(data.pipeline != NULL) {
        gst_element_call_async(data.pipeline,(GstElementCallAsyncFunc)ChangeStateHandler,(GstState*)stateNewPtr,NULL);
    } else {
        delete stateNewPtr;
    }
}



//--------------------------------------------------------------------------
void GstDisplay::ChangeStateSync(GstState stateNew) {

    GstState stateOld;

    if(data.pipeline != NULL) {
        gst_element_get_state(data.pipeline, &stateOld, NULL,1*GST_SECOND);

        if(stateOld != stateNew){
            GstStateChangeReturn ret = gst_element_set_state (data.pipeline, stateNew);
            if (ret == GST_STATE_CHANGE_FAILURE) {
                qWarning() << "Error: cannot change state!";
            }
        }
    }
}



//--------------------------------------------------------------------------
void GstDisplay::SetupAppsrc(QSize size) {

    if(data.pipeline != NULL) {

        GstCaps *caps = gst_caps_new_simple ("video/x-raw",
                                             "format", G_TYPE_STRING, "RGB",
                                             "framerate", GST_TYPE_FRACTION, 30, 1,
                                             "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                                             "width", G_TYPE_INT, size.width(),
                                             "height", G_TYPE_INT, size.height(),
                                             (char*)NULL);

        g_object_set (G_OBJECT (data.appsrc),
                      "stream-type", 0,
                      "emit-signals", FALSE,
                      "caps", caps,
                      "format", GST_FORMAT_TIME, (char*)NULL);

        gst_caps_unref (caps);
    }
}



//--------------------------------------------------------------------------
void GstDisplay::PushFrame(QImage image) {

    if(data.pipeline != NULL) {

        GstBuffer *buffer;
        GstFlowReturn ret;
        GstMapInfo map;

        int depth = 3;

        // Create a new empty buffer
        buffer = gst_buffer_new_allocate  (NULL, image.width()*image.height()*depth, NULL);

        if(gst_buffer_map (buffer, &map, GST_MAP_WRITE)) {

            //memset (map.data, 0xff, map.size); // All white --> ok
            memcpy (map.data, image.bits(), map.size);
        }

        // Push the buffer into the appsrc
        g_signal_emit_by_name (data.appsrc, "push-buffer", buffer, &ret);

        // Free the buffer now that we are done with it
        gst_buffer_unmap (buffer, &map);
        gst_buffer_unref (buffer);

        // We got some error, stop sending data
        if (ret != GST_FLOW_OK) {
            qDebug() << "GST_FLOW Error!" << ret;
        }
    }
}



//--------------------------------------------------------------------------
GstState GstDisplay::GetState() {

    GstState state = GST_STATE_NULL;

    if(data.pipeline != NULL) {

        gst_element_get_state(data.pipeline, &state, NULL,1*GST_SECOND);
    }
    return state;
}



//--------------------------------------------------------------------------
void GstDisplay::GstPlay(bool sync) {

    if (sync)  {
        ChangeStateSync(GST_STATE_PLAYING);
    } else {
        ChangeStateAsync(GST_STATE_PLAYING);
    }
}



//--------------------------------------------------------------------------
void GstDisplay::GstStop(bool sync) {

    if (sync)  {
        ChangeStateSync(GST_STATE_NULL);
    } else {
        ChangeStateAsync(GST_STATE_NULL);
    }
}



//--------------------------------------------------------------------------
void GstDisplay::GstPause(bool sync) {

    if (sync)  {
        ChangeStateSync(GST_STATE_PAUSED);
    } else {
        ChangeStateAsync(GST_STATE_PAUSED);
    }
}



//-------------------------------------------------------------------------------------------------
void GstDisplay::OnPainted(QImage image) {

    qDebug() << image;

    if(data.pipeline == NULL) {
        InstantiatePipeline(image.size());
        GstPlay();
    }

    PushFrame(image);
}



//-------------------------------------------------------------------------------------------------
void GstDisplay::OnResized(QSize size) {

    qDebug() << size;

    /*
    if(data.pipeline == NULL) {
        InstantiatePipeline();
    }

    GstStop(true);

    SetWidth(size.width());
    SetHeight(size.height());
    SetDepth(3);

    // Setup appsrc
    GstCaps *caps = gst_caps_new_simple ("video/x-raw",
                                         "format", G_TYPE_STRING, "RGB",
                                         "framerate", GST_TYPE_FRACTION, 30, 1,
                                         "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                                         "width", G_TYPE_INT, width,
                                         "height", G_TYPE_INT, height,
                                         (char*)NULL);

    g_object_set (G_OBJECT (data.appsrc),
                  "stream-type", 0,
                  "emit-signals", FALSE,
                  "caps", caps,
                  "format", GST_FORMAT_TIME, (char*)NULL);

    gst_caps_unref (caps);

    GstPlay(true);
    */

    Q_UNUSED(size);
}


