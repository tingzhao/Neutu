#include "zflyembodyannotation.h"

#include <iostream>
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "zstring.h"

const char *ZFlyEmBodyAnnotation::m_bodyIdKey = "body ID";
const char *ZFlyEmBodyAnnotation::m_nameKey = "name";
const char *ZFlyEmBodyAnnotation::m_typeKey = "class";
const char *ZFlyEmBodyAnnotation::m_commentKey = "comment";
const char *ZFlyEmBodyAnnotation::m_statusKey = "status";

ZFlyEmBodyAnnotation::ZFlyEmBodyAnnotation() : m_bodyId(0)
{
}

/*member dependent*/
void ZFlyEmBodyAnnotation::clear()
{
  m_bodyId = 0;
  m_status.clear();
  m_comment.clear();
  m_name.clear();
  m_type.clear();
}

void ZFlyEmBodyAnnotation::loadJsonString(const std::string &str)
{
  clear();

  ZJsonObject obj;
  obj.decodeString(str.c_str());

  loadJsonObject(obj);
}

/*member dependent*/
ZJsonObject ZFlyEmBodyAnnotation::toJsonObject() const
{
  ZJsonObject obj;
  if (m_bodyId > 0) {
    obj.setEntry(m_bodyIdKey, m_bodyId);

    if (!m_name.empty()) {
      obj.setEntry(m_nameKey, m_name);
    }

    if (!m_type.empty()) {
      obj.setEntry(m_typeKey, m_type);
    }

    if (!m_status.empty()) {
      obj.setEntry(m_statusKey, m_status);
    }

    if (!m_comment.empty()) {
      obj.setEntry(m_commentKey, m_comment);
    }
  }

  return obj;
}

/*member dependent*/
void ZFlyEmBodyAnnotation::loadJsonObject(const ZJsonObject &obj)
{
  if (obj.hasKey(m_bodyIdKey) || obj.hasKey(m_statusKey) ||
      obj.hasKey(m_commentKey) || obj.hasKey(m_nameKey) ||
      obj.hasKey(m_typeKey)) {
    if (obj.hasKey(m_bodyIdKey)) {
      setBodyId(ZJsonParser::integerValue(obj[m_bodyIdKey]));
    }

    if (obj.hasKey(m_statusKey)) {
      setStatus(ZJsonParser::stringValue(obj[m_statusKey]));
    }

    if (obj.hasKey(m_commentKey)) {
      setComment(ZJsonParser::stringValue(obj[m_commentKey]));
    }

    if (obj.hasKey(m_nameKey)) {
      setName(ZJsonParser::stringValue(obj[m_nameKey]));
    }

    if (obj.hasKey(m_typeKey)) {
      setType(ZJsonParser::stringValue(obj[m_typeKey]));
    }
  } else {
    std::vector<std::string> keyList = obj.getAllKey();
    if (keyList.size() == 1) {
      int bodyId = ZString(keyList.front()).firstInteger();
      if (bodyId > 0) {
        setBodyId(bodyId);
        ZJsonObject annotationJson(
            const_cast<json_t*>(obj[keyList.front().c_str()]),
            ZJsonObject::SET_INCREASE_REF_COUNT);
        setType(ZJsonParser::stringValue(annotationJson["Type"]));
        setName(ZJsonParser::stringValue(annotationJson["Name"]));
      }
    }
  }
}

/*member dependent*/
void ZFlyEmBodyAnnotation::print() const
{
  std::cout << "Body annotation:" << std::endl;
  std::cout << "  Body ID: " << m_bodyId << std::endl;
  std::cout << "  Type: " << m_type << std::endl;
  std::cout << "  Name: " << m_name << std::endl;
  std::cout << "  Status: " << m_status << std::endl;
  std::cout << "  Comment: " << m_comment << std::endl;
}

/*member dependent*/
bool ZFlyEmBodyAnnotation::isEmpty() const
{
  return m_status.empty() && m_comment.empty() && m_name.empty() &&
      m_type.empty();
}
