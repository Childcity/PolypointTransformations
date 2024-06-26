#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
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
