#include <QGuiApplication>
#include <QOpenGLContext>
#include <QQmlApplicationEngine>
#include <Qt3DRender/qt3drender-config.h>

void setupSurfaceFormat()
{
	QSurfaceFormat format;
#if QT_CONFIG(opengles2)
	format.setRenderableType(QSurfaceFormat::OpenGLES);
#else
	if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
		// 4.6 - Latest on my PC. Driver ver. 30.0.101.1994 (UHD Graphics 770)
		format.setVersion(4, 6);
		format.setProfile(QSurfaceFormat::CoreProfile);
	}
#endif
	format.setDepthBufferSize(24);
	format.setSamples(4);
	format.setStencilBufferSize(8);
	QSurfaceFormat::setDefaultFormat(format);

	qputenv("QT3D_RENDERER", "rhi"); // rhi / opengl
	qputenv("QSG_RHI_BACKEND", "opengl"); // vulkan, metal, opengl, d3d11, d3d12
	qputenv("QSG_RHI_DEBUG_LAYER", "1");
	// qputenv("QSG_INFO", "1");
	// qputenv("QT_RHI_LEAK_CHECK", "0");
}

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	setupSurfaceFormat();

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
