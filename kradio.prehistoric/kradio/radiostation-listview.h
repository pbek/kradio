
#ifdef NOT_YET_READY_FOR_COMPILATION

#ifndef _KRADIO_RADIOSTATION_LISTVIEW_H_
#define _KRADIO_RADIOSTATION_LISTVIEW_H_

#include <klistview.h>

class RadioStationListView : public KListView
{
public:
    RadioStationListView(QWidget *parent=0, const char *name=0);
    RadioStationListView();

    setItem(RadioStation *, idx);
    setStations(StationList);
protected:

};

#endif


#endif
