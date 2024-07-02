#include <QGuiApplication>
#include <QOpenGLContext>
#include <QQmlApplicationEngine>
#include <Qt3DRender/qt3drender-config.h>

void setSurfaceFormat()
{
	QSurfaceFormat format;
#if QT_CONFIG(opengles2)
	format.setRenderableType(QSurfaceFormat::OpenGLES);
#else
	if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
		format.setVersion(4, 3);
		format.setProfile(QSurfaceFormat::CoreProfile);
	}
#endif
	format.setDepthBufferSize(24);
	format.setSamples(4);
	format.setStencilBufferSize(8);
	QSurfaceFormat::setDefaultFormat(format);

#if !QT_CONFIG(qt3d_rhi_renderer)
	qputenv("QSG_RHI_BACKEND", "opengl");
#endif
}

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	// qputenv("QT3D_RENDERER", "opengl");
	qputenv("QSG_RHI_BACKEND", "opengl");
	// qputenv("QSG_RHI_PREFER_SOFTWARE_RENDERER", "1");
	setSurfaceFormat();

	app.setOrganizationName("Childcity");
	app.setOrganizationDomain("childcity.com");
	app.setApplicationName("Polydot Transformations");

	QQmlApplicationEngine engine;
	QObject::connect(
	    &engine,
	    &QQmlApplicationEngine::objectCreationFailed,
	    &app,
	    []() { QCoreApplication::exit(-1); },
	    Qt::QueuedConnection);

	engine.loadFromModule("PolydotTransformationUi", "Main");

	return app.exec();
}
