#include "zdvidurl.h"
#include <sstream>
#include "dvid/zdviddata.h"
#include "zstring.h"

ZDvidUrl::ZDvidUrl()
{
}

ZDvidUrl::ZDvidUrl(
    const std::string &serverAddress, const std::string &uuid, int port)
{
  m_dvidTarget.set(serverAddress, uuid, port);
}

ZDvidUrl::ZDvidUrl(const ZDvidTarget &target)
{
  m_dvidTarget = target;
}

std::string ZDvidUrl::getNodeUrl() const
{
  return m_dvidTarget.getUrl();
}

std::string ZDvidUrl::getDataUrl(const std::string &dataName) const
{
  return m_dvidTarget.getUrl() + "/" + dataName;
}

std::string ZDvidUrl::getDataUrl(ZDvidData::ERole role) const
{
  return getDataUrl(ZDvidData::getName(role));
}

std::string ZDvidUrl::getInfoUrl(const std::string &dataName) const
{
  return getDataUrl(dataName) + "/info";
}

std::string ZDvidUrl::getApiUrl() const
{
  return  "http://" + m_dvidTarget.getAddressWithPort() + "/api";
}

std::string ZDvidUrl::getHelpUrl() const
{
  return getApiUrl() + "/help";
}

std::string ZDvidUrl::getServerInfoUrl() const
{
  return getApiUrl() + "/server/info";
}

std::string ZDvidUrl::getSkeletonUrl() const
{
  return getDataUrl(ZDvidData::getName(ZDvidData::ROLE_SKELETON));
}

std::string ZDvidUrl::getSkeletonUrl(int bodyId) const
{
  if (bodyId < 0) {
    return "";
  }

  ZString str;
  str.appendNumber(bodyId);

  return getSkeletonUrl() + "/" + str + ".swc";
}

std::string ZDvidUrl::getSparsevolUrl() const
{
  return getDataUrl(ZDvidData::ROLE_SP2BODY) + "/" +
      ZDvidData::getName(ZDvidData::ROLE_SPARSEVOL);
}

std::string ZDvidUrl::getSparsevolUrl(int bodyId) const
{
  if (bodyId < 0) {
    return "";
  }

  ZString str;
  str.appendNumber(bodyId);

  return getSparsevolUrl() + "/" + str;
}

std::string ZDvidUrl::getThumbnailUrl() const
{
  return getDataUrl(ZDvidData::getName(ZDvidData::ROLE_THUMBNAIL));
}

std::string ZDvidUrl::getThumbnailUrl(int bodyId) const
{
  if (bodyId < 0) {
    return "";
  }

  ZString str;
  str.appendNumber(bodyId);

  return getThumbnailUrl() + "/" + str + ".mraw";
}

std::string ZDvidUrl::getRepoUrl() const
{
  return getApiUrl() + "/repo/" + m_dvidTarget.getUuid();
}

std::string ZDvidUrl::getInstanceUrl() const
{
  return getRepoUrl() + "/instance";
}


std::string ZDvidUrl::getSp2bodyUrl() const
{
  return getDataUrl(ZDvidData::getName(ZDvidData::ROLE_SP2BODY));
}

std::string ZDvidUrl::getSp2bodyUrl(const std::string &suffix) const
{
  return getSp2bodyUrl() + "/" + suffix;
}

std::string ZDvidUrl::getGrayscaleUrl() const
{
  return getDataUrl(ZDvidData::getName(ZDvidData::ROLE_GRAY_SCALE));
}

std::string ZDvidUrl::getGrayscaleUrl(int sx, int sy, int x0, int y0, int z0)
const
{
  std::ostringstream stream;
  stream << "/raw/0_1" << sx << "_" << sy << "/" << x0 << "_" << y0 << "_" << z0;
  return getGrayscaleUrl() + stream.str();
}

std::string ZDvidUrl::getGrayscaleUrl(int sx, int sy, int sz,
                                      int x0, int y0, int z0) const
{
  std::ostringstream stream;
  stream << "/raw/0_1_2/" << sx << "_" << sy << "_" << sz << "/"
         << x0 << "_" << y0 << "_" << z0;
  return getGrayscaleUrl() + stream.str();
}

std::string ZDvidUrl::getLabels64Url(
    const std::string &name, int sx, int sy, int sz,
    int x0, int y0, int z0) const
{
  std::ostringstream stream;
  stream << "/raw/0_1_2/" << sx << "_" << sy << "_" << sz << "/"
         << x0 << "_" << y0 << "_" << z0;
  return getDataUrl(name) + stream.str();
}

std::string ZDvidUrl::getKeyUrl(const std::string &name, const std::string &key) const
{
  return getDataUrl(name) + "/" + key;
}

std::string ZDvidUrl::getKeyRangeUrl(
    const std::string &name,
    const std::string &key1, const std::string &key2) const
{
  return getDataUrl(name) + "/" + key1 + "/" + key2;
}

std::string ZDvidUrl::getAnnotationUrl() const
{
  return getDataUrl(ZDvidData::getName(ZDvidData::ROLE_BODY_ANNOTATION));
}

std::string ZDvidUrl::getAnnotationUrl(int bodyId) const
{
  return getAnnotationUrl() + "/" + ZString::num2str(bodyId);
}

std::string ZDvidUrl::getBoundBoxUrl() const
{
  return getDataUrl(ZDvidData::ROLE_BOUND_BOX);
}

std::string ZDvidUrl::getBoundBoxUrl(int z) const
{
  return getBoundBoxUrl() + "/" + ZString::num2str(z);
}