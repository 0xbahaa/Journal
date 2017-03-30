#include "papermodel.h"

QJsonObject PaperModel::itemToVariant(QGraphicsItem* item, QUuid id)
{
	QJsonObject data;

	data["id"] = id.toString();
	data["posx"] = item->pos().x();
	data["posy"] = item->pos().y();

	if (QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(item))
	{
		data["type"] = "pixmap";
		data["pixmap"] = jsonValFromPixmap(pixmapItem->pixmap()); //QJsonValue::fromVariant(pixmapItem->pixmap());
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
		data["path"] = QVariant(getPointsInPath(pathItem->path())).toJsonArray();
	}
	else return QJsonObject();

	return data;
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

QList<QVariant> PaperModel::getPointsInPath(QPainterPath path)
{
	QList<QVariant> points;
	for(int i = 0; i < path.elementCount(); i++)
		points.append(QVariant(QPointF(path.elementAt(i).x, path.elementAt(i).y)));
	return points;
}

QPainterPath PaperModel::getPathFromJson(QJsonValue json)
{
	QList<QVariant> points = json.toArray().toVariantList();
	return getPathFromPoints(points, points.first().toPointF());
}

QPainterPath PaperModel::getPathFromPoints(QList<QVariant> points, QPointF position)
{
	QPainterPath path(position);

	foreach(QVariant point, points)
		path.lineTo(point.toPointF());

	return path;
}

PaperModel::PaperModel() : appDirectoryName("/journal/")
{
	QString configLocation = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
	appDirectoryLocation = configLocation + appDirectoryName;
	qDebug() << appDirectoryLocation;

	QDir dir;
	if(!dir.exists(appDirectoryLocation))
		dir.mkdir(appDirectoryLocation);
}

Paper *PaperModel::loadPaper(QString path)
{
	Paper* paper = new Paper();
	paper->setPaperID(path);
	QFile paperFile(appDirectoryLocation + path);
	paperFile.open(QIODevice::ReadOnly);

	QByteArray byteArray = paperFile.readAll();

	QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray));

	foreach( QJsonValue data, jsonDocument.array() )
		paper->addSavableItem(itemFromJson(data.toObject()), QUuid(data.toObject()["id"].toString()));

	return paper;
}

void PaperModel::savePaper(Paper *paper)
{
	QVariantList data_list;

	QHashIterator<QGraphicsItem*, QUuid> i(paper->savableItems);
	while (i.hasNext())
	{
		i.next();
		data_list << itemToVariant(i.key(), i.value());
	}

	QFile paperFile(appDirectoryLocation + paper->id.toString());
	paperFile.open(QIODevice::WriteOnly);
	QJsonDocument jsonDocument;
	jsonDocument = QJsonDocument::fromVariant(data_list);
	paperFile.write(jsonDocument.toJson());
	paperFile.close();
}
