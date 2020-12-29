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

    GstElement* filesrc;
    GstElement* decodebin;
    GstElement* audioconvert;
    GstElement* videosink;

    GstDisplay* pOwner;

} GstAudioPlayerData;



//-------------------------------------------------------------------------------------------------
class GstDisplay : public QObject {

    Q_OBJECT

public:
    explicit GstDisplay(QObject *parent = nullptr);
    ~GstDisplay();

    void InstantiatePipeline();
    void DisposePipeline();

    void ChangeStateAsync(GstState stateNew);
    void ChangeStateSync(GstState stateNew);

    GstState GetState();

    void GstPlay(bool sync = false);
    void GstStop(bool sync = false);
    void GstPause(bool sync = false);

public slots:
    void OnPainted(QImage image);

private:
    GstAudioPlayerData data;
    guint busWatchId;

    QImage image;
};



#endif // GSTDISPLAY_H
