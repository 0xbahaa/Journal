#ifndef PAPERMODEL_H
#define PAPERMODEL_H

#include <QObject>

#include "paper.h"

class PaperModel : public QThread
{
	Q_OBJECT

private:
	const QString appDirectoryName;
	QString appDirectoryLocation;
	QJsonObject* paperJson;
	Paper* paper;
	QFile* paperFile;

	QJsonValue itemToJson(QGraphicsItem* item);
	QGraphicsItem* itemFromJson(QJsonObject data, QString mediaFileLocation);
	QPainterPath getPathFromJson(QJsonValue json);
	QPainterPath getPathFromPoints(QList<QVariant> points);
	QList<QPointF> getPointsInPath(QPainterPath path);
	QJsonArray jsonArrayFromPath(QPainterPath path);
	void saveToFile(QJsonDocument jsonDocument);
	QString copyMediaFileToJournal(QUuid id, QString path);
	QString getAbsolutePath(QString relativePath);
	QGraphicsPixmapItem* pixmap_cast(QGraphicsItem* item);
	QGraphicsPathItem* pathItem_cast(QGraphicsItem* item);
	QGraphicsSimpleTextItem* simpleText_cast(QGraphicsItem* item);
	QGraphicsTextItem* text_cast(QGraphicsItem* item);

public slots:
	void onItemModified(QUuid id, QGraphicsItem* item, QString itemPath);

protected:
	void run() override;

signals:

public:
	PaperModel(QString path);
	~PaperModel();

	Paper* loadPaper(QString path);
	void savePaper(Paper* paper);
	Paper *getPaper() const;
};

#endif // PAPERMODEL_H
