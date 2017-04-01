#include "papermodel.h"

QJsonValue PaperModel::itemToJson(QGraphicsItem* item)
{
	QJsonObject data;

	data["posx"] = item->pos().x();
	data["posy"] = item->pos().y();

	if (QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(item))
	{
		data["type"] = "pixmap";
		data["pixmap"] = jsonValFromPixmap(pixmapItem->pixmap());
	}
	else if (QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item))
	{
		data["type"] = "richtext";
		data["document"] = textItem->document()->toHtml();
	}
	else if (QGraphicsSimpleTextItem* simpleTextItem = dynamic_cast<QGraphicsSimpleTextItem*>(item))
	{
		data["type"] = "simpletext";
		data["text"] = simpleTextItem->text();
	}
	else if (QGraphicsPathItem* pathItem = dynamic_cast<QGraphicsPathItem*>(item))
	{
		data["type"] = "stroke";
		data["path"] = jsonArrayFromPath(pathItem->path());
	}
	else return QJsonValue();

	return QJsonValue(data);
}

QGraphicsItem* PaperModel::itemFromJson(QJsonObject data)
{
	QGraphicsItem* result;

	if (data["type"] == "pixmap")
		result = new QGraphicsPixmapItem(pixmapFrom(data["pixmap"]));
	else if(data["type"] == "stroke")
		result = new QGraphicsPathItem(getPathFromJson(data["path"]));
	else if(data["type"] == "simpletext")
		result = new QGraphicsSimpleTextItem(data["text"].toString());
	else if(data["type"] == "richtext")
	{
		result = new QGraphicsTextItem();
		((QGraphicsTextItem *)result)->setHtml(data["document"].toString());
	}
	else return nullptr;

	result->setPos(data["posx"].toDouble(), data["posy"].toDouble());
	return result;
}

QJsonValue PaperModel::jsonValFromPixmap(const QPixmap & p)
{
	QByteArray data;
	QBuffer buffer { &data };
	buffer.open(QIODevice::WriteOnly);
	p.save(&buffer, "PNG");
	auto encoded = buffer.data().toBase64();
	return QJsonValue(QString::fromLatin1(encoded));
}

void PaperModel::onItemModified(QUuid id, QGraphicsItem *item)
{
	QJsonObject::Iterator i = paperJson->find(id.toString());
	QJsonValue itemJson = itemToJson(item);

	if(i == paperJson->end())
		paperJson->insert(id.toString(), itemJson);
	else
		i.value() = itemJson;
	saveToFile(QJsonDocument(*paperJson));
}

void PaperModel::run()
{

}

void PaperModel::saveToFile(QJsonDocument jsonDocument)
{
	paperFile.open(QIODevice::WriteOnly);
	paperFile.write(jsonDocument.toJson());
	paperFile.close();
}

QPixmap PaperModel::pixmapFrom(const QJsonValue & val)
{
	QByteArray encoded = val.toString().toLatin1();
	QPixmap p;
	p.loadFromData(QByteArray::fromBase64(encoded), "PNG");
	return p;
}

bool PaperModel::dataIs(QVariantHash data, QString type)
{
	return data["type"].toString() == type;
}

QList<QPointF> PaperModel::getPointsInPath(QPainterPath path)
{
	QList<QPointF> points;
	for(int i = 0; i < path.elementCount(); i++)
		points.append(QPointF(path.elementAt(i).x, path.elementAt(i).y));
	return points;
}

QJsonArray PaperModel::jsonArrayFromPath(QPainterPath path)
{
	QList<QPointF> points = getPointsInPath(path);
	QVariantList pointPositions;


	foreach(QPointF point, points)
	{
		pointPositions.append(QVariant(point.x()));
		pointPositions.append(QVariant(point.y()));
	}

	QJsonArray tmp = QJsonArray::fromVariantList(pointPositions);
	return tmp;
}

QPainterPath PaperModel::getPathFromJson(QJsonValue json)
{
	QList<QVariant> points = json.toArray().toVariantList();
	return getPathFromPoints(points);
}

QPainterPath PaperModel::getPathFromPoints(QList<QVariant> points)
{
	QPointF position(points[0].toDouble(), points[1].toDouble());
	QPainterPath path(position);

	// it's a list of [x, y, x, y, x, y...]
	for (int i = 2; i < points.size() - 1; i+=2)
	{
		QPointF point(points[i].toDouble(), points[i+1].toDouble());
		path.lineTo(point);
	}

	return path;
}

PaperModel::PaperModel(QObject* parent) : QThread(parent), appDirectoryName("/journal/")
{
	QString configLocation = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
	appDirectoryLocation = configLocation + appDirectoryName;

	QDir dir;
	if(!dir.exists(appDirectoryLocation))
		dir.mkdir(appDirectoryLocation);

	paperFile.setFileName(appDirectoryLocation + paper->id.toString());
}

PaperModel::~PaperModel()
{
	paperFile.close();
}

Paper *PaperModel::loadPaper(QString path)
{
	paper = new Paper();

	paper->setPaperID(path);

	paperFile.open(QIODevice::ReadOnly);

	QByteArray byteArray = paperFile.readAll();

	paperFile.close();

	QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray));
	paperJson = new QJsonObject(jsonDocument.object());

	QJsonObject::Iterator i;

	for(i = paperJson->begin(); i != paperJson->end(); i++)
		paper->addSavableItem(itemFromJson(i.value().toObject()), QUuid(i.key()));

	return paper;
}

void PaperModel::savePaper(Paper *paper)
{
	this->paper = paper;
	paperJson = new QJsonObject();

	QHashIterator<QGraphicsItem*, QUuid> i(paper->savableItems);
	while (i.hasNext())
	{
		i.next();
		paperJson->insert(i.value().toString(), itemToJson(i.key()));
	}

	QJsonDocument jsonDocument(*paperJson);

	saveToFile(jsonDocument);
}
