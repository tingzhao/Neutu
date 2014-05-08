#ifndef ZFLYEMDVIDREADER_H
#define ZFLYEMDVIDREADER_H

#include "dvid/zdvidreader.h"
#include "zflyembodyannotation.h"

/*!
 * \brief The class of reading flyem data from DVID
 */
class ZFlyEmDvidReader : public ZDvidReader
{
public:
  ZStack* readThumbnail(int bodyId);
  ZFlyEmBodyAnnotation readAnnotation(int bodyId);
};

#endif // ZFLYEMDVIDREADER_H
