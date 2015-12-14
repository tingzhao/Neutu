#include "zdvidsynapse.h"
#include <QtCore>
#include "zpainter.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zjsonfactory.h"
#include "zjsonarray.h"

ZDvidSynapse::ZDvidSynapse()
{
  init();
}

void ZDvidSynapse::init()
{
  m_projectionVisible = false;
  m_kind = KIND_INVALID;
  setDefaultRadius();
}

void ZDvidSynapse::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/) const
{
  bool visible = true;

  if (slice < 0) {
    visible = isProjectionVisible();
  } else {
    int z = painter.getZ(slice);
    visible = (z == m_position.getZ());
  }

  if (visible) {
    painter.setPen(getColor());
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPointF(m_position.getX(), m_position.getY()),
                        m_radius, m_radius);
  }

  QPen pen = painter.getPen();

  bool drawingBoundBox = false;
  if (isSelected()) {
    drawingBoundBox = true;
    QColor color;
    color.setRgb(255, 255, 0);
    pen.setColor(color);
    pen.setCosmetic(true);
  } else if (hasVisualEffect(NeuTube::Display::Sphere::VE_BOUND_BOX)) {
    drawingBoundBox = true;
    pen.setStyle(Qt::SolidLine);
    pen.setCosmetic(m_usingCosmeticPen);
  }

  if (drawingBoundBox) {
    QRectF rect;
    double halfSize = m_radius;
    if (m_usingCosmeticPen) {
      halfSize += 0.5;
    }
    rect.setLeft(m_position.getX() - halfSize);
    rect.setTop(m_position.getY() - halfSize);
    rect.setWidth(halfSize * 2);
    rect.setHeight(halfSize * 2);

    painter.setBrush(Qt::NoBrush);
    pen.setWidthF(pen.widthF() * 0.5);
    if (visible) {
      pen.setStyle(Qt::SolidLine);
    } else {
      pen.setStyle(Qt::DotLine);
    }
    painter.setPen(pen);
    painter.drawRect(rect);
  }

  if (isSelected()) {
    for (std::vector<ZIntPoint>::const_iterator iter = m_partnerHint.begin();
         iter != m_partnerHint.end(); ++iter) {
      ZIntPoint pos = *iter;
      painter.drawLine(getPosition().getX(), getPosition().getY(),
                       pos.getX(), pos.getY());
    }
  }
}

void ZDvidSynapse::setPosition(const ZIntPoint &pos)
{
  m_position = pos;
}

void ZDvidSynapse::setPosition(int x, int y, int z)
{
  m_position.set(x, y, z);
}

void ZDvidSynapse::setDefaultRadius()
{
  switch (m_kind) {
  case KIND_POST_SYN:
    setRadius(3.0);
    break;
  case KIND_PRE_SYN:
    setRadius(5.0);
    break;
  default:
    setRadius(5.0);
    break;
  }
}

void ZDvidSynapse::setDefaultColor()
{
  switch (m_kind) {
  case KIND_POST_SYN:
    setColor(0, 0, 255);
    break;
  case KIND_PRE_SYN:
    setColor(0, 255, 0);
    break;
  default:
    setColor(0, 255, 255);
    break;
  }
}

bool ZDvidSynapse::hit(double x, double y, double z)
{
  return m_position.distanceTo(x, y, z) <= m_radius;
}

bool ZDvidSynapse::hit(double x, double y)
{
  double dx = x - m_position.getX();
  double dy = y = m_position.getY();

  double d2 = dx * dx * dy * dy;

  return d2 <= m_radius * m_radius;
}

void ZDvidSynapse::clear()
{
  m_position.set(0, 0, 0);
  m_kind = KIND_INVALID;
  m_tagArray.clear();
  setDefaultRadius();
}

bool ZDvidSynapse::isValid() const
{
  return getKind() != KIND_INVALID;
}

void ZDvidSynapse::loadJsonObject(const ZJsonObject &obj)
{
  clear();
  if (obj.hasKey("Pos")) {
    json_t *value = obj.value("Pos").getData();
    m_position.setX(ZJsonParser::integerValue(value, 0));
    m_position.setY(ZJsonParser::integerValue(value, 1));
    m_position.setZ(ZJsonParser::integerValue(value, 2));

    if (obj.hasKey("Kind")) {
      setKind(ZJsonParser::stringValue(obj["Kind"]));
    } else {
      setKind(KIND_UNKNOWN);
    }

    if (obj.hasKey("Tags")) {
      ZJsonArray tagJson(obj["Tags"], ZJsonValue::SET_INCREASE_REF_COUNT);
      for (size_t i = 0; i < tagJson.size(); ++i) {
        m_tagArray.push_back(ZJsonParser::stringValue(tagJson.at(i), i));
      }
    }

    setDefaultRadius();
    setDefaultColor();
  }
}

void ZDvidSynapse::setKind(const std::string &kind)
{
  if (kind == "PostSyn") {
    setKind(KIND_POST_SYN);
  } else if (kind == "PreSyn") {
    setKind(KIND_PRE_SYN);
  } else {
    setKind(KIND_UNKNOWN);
  }
}

std::ostream& operator<< (std::ostream &stream, const ZDvidSynapse &synapse)
{
  //"Kind": (x, y, z)
  switch (synapse.getKind()) {
  case ZDvidSynapse::KIND_POST_SYN:
    stream << "PostSyn";
    break;
  case ZDvidSynapse::KIND_PRE_SYN:
    stream << "PreSyn";
    break;
  default:
    stream << "Unknown";
    break;
  }

  stream << ": " << "(" << synapse.getPosition().getX() << ", "
         << synapse.getPosition().getY() << ", "
         << synapse.getPosition().getZ() << ")";

  return stream;
}

void ZDvidSynapse::clearPartner()
{
  m_partnerHint.clear();
}

void ZDvidSynapse::addPartner(int x, int y, int z)
{
  m_partnerHint.push_back(ZIntPoint(x, y, z));
}

void ZDvidSynapse::addTag(const std::string &tag)
{
  m_tagArray.push_back(tag);
}

std::string ZDvidSynapse::GetKindName(EKind kind)
{
  switch (kind) {
  case ZDvidSynapse::KIND_POST_SYN:
    return "PostSyn";
  case ZDvidSynapse::KIND_PRE_SYN:
    return "PreSyn";
  default:
    break;
  }

  return "Unknown";
}

ZDvidSynapse::EKind ZDvidSynapse::GetKind(const std::string &name)
{
  if (name == "PostSyn") {
    return ZDvidSynapse::KIND_POST_SYN;
  } else if (name == "PreSyn") {
    return ZDvidSynapse::KIND_PRE_SYN;
  }

  return ZDvidSynapse::KIND_UNKNOWN;
}

ZJsonObject ZDvidSynapse::makeRelJson(const ZIntPoint &pt) const
{
  std::string rel;
  switch (getKind()) {
  case ZDvidSynapse::KIND_POST_SYN:
    rel = "PostSynTo";
    break;
  case ZDvidSynapse::KIND_PRE_SYN:
    rel = "PreSynTo";
    break;
  default:
    rel = "UnknownRelationship";
  }

  ZJsonObject relJson;
  relJson.setEntry("Rel", rel);

  ZJsonArray posJson = ZJsonFactory::MakeJsonArray(pt);
  relJson.setEntry("To", posJson);

  return relJson;
}

ZJsonObject ZDvidSynapse::toJsonObject() const
{
  ZJsonObject obj;

  ZJsonArray posJson = ZJsonFactory::MakeJsonArray(m_position);
  obj.setEntry("Pos", posJson);
  obj.setEntry("Kind", GetKindName(getKind()));
  if (!m_partnerHint.empty()) {
    ZJsonArray relArrayJson;
    for (std::vector<ZIntPoint>::const_iterator iter = m_partnerHint.begin();
         iter != m_partnerHint.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      relArrayJson.append(makeRelJson(pt));
    }
    obj.setEntry("Rels", relArrayJson);
  }

  if (!m_tagArray.empty()) {
    ZJsonArray tagJson;
    for (std::vector<std::string>::const_iterator iter = m_tagArray.begin();
         iter != m_tagArray.end(); ++iter) {
      const std::string &tag = *iter;
      tagJson.append(tag);
    }
    obj.setEntry("Tag", tagJson);
  }

  return obj;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSynapse)
