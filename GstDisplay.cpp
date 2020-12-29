#include "GstDisplay.h"
#include <QDebug>

#include <gst/video/video.h>


QImage* image_ptr;



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



static void NeedDataHandler (GstElement *appsrc) {

    GstBuffer *buffer;
    GstFlowReturn ret;

    /* Create a new empty buffer */
    buffer = gst_buffer_new_and_alloc (300*300*4);

    /* Push the buffer into the appsrc */
    g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);

    /* Free the buffer now that we are done with it */
    gst_buffer_unref (buffer);

    if (ret != GST_FLOW_OK) {
        /* We got some error, stop sending data */
        qDebug() << "Error!";
    }
}



//-------------------------------------------------------------------------------------------------
GstDisplay::GstDisplay(QObject *parent) : QObject(parent) {

    data.pipeline = NULL;

    data.filesrc = NULL;
    data.decodebin = NULL;
    data.audioconvert = NULL;
    data.videosink = NULL;

    data.pOwner = this;

    busWatchId = -1;
}



//-------------------------------------------------------------------------------------------------
GstDisplay::~GstDisplay() {

    DisposePipeline();
}



//--------------------------------------------------------------------------
void GstDisplay::InstantiatePipeline() {

    if(data.pipeline == NULL) {

        data.pipeline = gst_pipeline_new ("pipeline");

        data.filesrc = gst_element_factory_make("appsrc", NULL);
        data.decodebin = gst_element_factory_make("identity", NULL);
        data.audioconvert = gst_element_factory_make("videoconvert", NULL);
        data.videosink = gst_element_factory_make("autovideosink", NULL);

        if (!data.pipeline || !data.filesrc || !data.decodebin || !data.audioconvert || !data.videosink) {
            qWarning() << "Not all elements could be created!";
            DisposePipeline();
            return;
        }

        gst_bin_add_many (GST_BIN(data.pipeline), data.filesrc, data.decodebin, data.audioconvert, data.videosink, (char*)NULL);

        // Link the pipeline
        if (!gst_element_link_many (data.filesrc, data.decodebin, data.audioconvert, data.videosink, (char*)NULL)) {
            qWarning() << "Error: not all elements could be linked!";
            DisposePipeline();
            return;
        }

        /* setup appsrc */
        GstVideoInfo info;
        GstCaps *video_caps;
        gst_video_info_set_format (&info, GST_VIDEO_FORMAT_ARGB,300, 300);
        video_caps = gst_video_info_to_caps (&info);
        g_object_set (data.filesrc, "caps", video_caps, "format", GST_FORMAT_TIME, (char*)NULL);

        g_object_set (G_OBJECT (data.filesrc),
                      "stream-type", 0,
                      "format", GST_FORMAT_TIME, (char*)NULL);
        g_signal_connect (data.filesrc, "need-data", G_CALLBACK (NeedDataHandler), NULL);
    }
}



//--------------------------------------------------------------------------
void GstDisplay::DisposePipeline() {

    if(data.pipeline != NULL) {

        GstStop(true);

        gst_object_unref (data.pipeline);

        data.pipeline = NULL;

        data.filesrc = NULL;
        data.decodebin = NULL;
        data.audioconvert = NULL;
        data.videosink = NULL;

        data.pOwner = NULL;

        busWatchId = -1;
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

    this->image = image;

    // Da migliorare
    image_ptr = &this->image;
    GstPlay();

    qDebug() << this->image;
}
