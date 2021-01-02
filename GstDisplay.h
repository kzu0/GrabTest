#ifndef GSTDISPLAY_H
#define GSTDISPLAY_H

#include <QObject>
#include <QImage>
#include "gst/gst.h"



//-------------------------------------------------------------------------------------------------------------
class GstDisplay;
//-------------------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------------------
typedef struct {
    GstElement* pipeline;

    GstElement* appsrc;
    GstElement* queue;
    GstElement* videoconvert;
    GstElement* videosink;

    GstDisplay* pOwner;

    GMainLoop *main_loop;  /* GLib's Main Loop */

} GstAudioPlayerData;



//-------------------------------------------------------------------------------------------------
class GstDisplay : public QObject {

    Q_OBJECT

public:
    explicit GstDisplay(QObject *parent = nullptr);
    ~GstDisplay();

    void InstantiatePipeline();
    void DisposePipeline();

    void SetWidth(int width);
    void SetHeight(int height);
    void SetDepth(int depth);

    GstState GetState();

    void GstPlay(bool sync = false);
    void GstStop(bool sync = false);
    void GstPause(bool sync = false);

public slots:
    void OnPainted(QImage image);

private:
    void ChangeStateAsync(GstState stateNew);
    void ChangeStateSync(GstState stateNew);

    void PushFrame(QImage image);

    GstAudioPlayerData data;

    int width;
    int height;
    int depth;

};



#endif // GSTDISPLAY_H
