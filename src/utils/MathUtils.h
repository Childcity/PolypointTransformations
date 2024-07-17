#pragma once

#include <QCamera>
#include <QObject>
#include <Qt3DInput>

#include <geometry/GeometryPrimitives.h>

class MathUtils : public QObject
{
	Q_OBJECT
	QML_SINGLETON
	QML_NAMED_ELEMENT(MathUtils)
	Q_DISABLE_COPY(MathUtils)

public:
	static MathUtils &Get();
	static QObject *Get(class QQmlEngine *, class QJSEngine *);

	explicit MathUtils(QObject * = nullptr);
	~MathUtils() override = default;

	static StreightLine getPolydotTransformedLine(
	    const Line &line, //
	    const QVariantList &origBasises,
	    const QVariantList &resBasises);

	static Mesh getPolydotTransformedMesh(
	    Mesh mesh, //
	    const QVariantList &origBasises,
	    const QVariantList &resBasises);

	static Mesh getPolydotTransformedStreightLineMesh(
	    Mesh mesh, //
	    const QVariantList &origBasises,
	    const QVariantList &resBasises);

	Q_INVOKABLE static QVector3D mouseEventToSpace(
	    const Qt3DInput::QMouseEvent *mouseEvent,
	    const Qt3DRender::QCamera *camera,
	    QSize surfaceSize);
};
