#pragma once

#include <geometry/GeometryPrimitives.h>

struct QXmlStreamReader;

class ColladaFormatImporter
{
public:
	explicit ColladaFormatImporter(const QString path);

	void importGeometries();
	MeshList getGeometries();

private:
	void readCollada(QXmlStreamReader &reader);
	void readLibraryGeometries(QXmlStreamReader &reader);
	void readMesh(QXmlStreamReader &reader);

private:
	QString m_path;
	MeshList m_meshes;
};
