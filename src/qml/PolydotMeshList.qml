import Qt3D.Core
import Qt3D.Extras
import Qt3D.Render

Entity {
	id: root

	property alias model: meshes.model
	property color color: "black"

	NodeInstantiator {
		id: meshes

		asynchronous: true

		delegate: PolydotMesh {
			//required property QtObject linesMeshModel

			model: linesMeshModel
			color: root.color
		}
	}
}
