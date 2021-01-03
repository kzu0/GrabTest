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

} GstAudioPlayerData;



//-------------------------------------------------------------------------------------------------
class GstDisplay : public QObject {

    Q_OBJECT

public:
    explicit GstDisplay(QObject *parent = nullptr);
    ~GstDisplay();

    void InstantiatePipeline(QSize size);
    void DisposePipeline();

    GstState GetState();

    void GstPlay(bool sync = false);
    void GstStop(bool sync = false);
    void GstPause(bool sync = false);

public slots:
    void OnPainted(QImage image);
    void OnResized(QSize size);

private:
    void ChangeStateAsync(GstState stateNew);
    void ChangeStateSync(GstState stateNew);

    void SetupAppsrc(QSize size);

    void PushFrame(QImage image);

    GstAudioPlayerData data;
};



#endif // GSTDISPLAY_H
