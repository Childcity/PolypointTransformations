import Qt3D.Core
import Qt3D.Extras
import Qt3D.Render

import PolydotTransformationUi

Entity {
	id: root

	property alias text: label.text
	property alias pos: tr.translation
	property bool isLight: true

	Transform {
		id: tr
		scale: 0.5
	}

	ObjectPicker {
		id: picker
		dragEnabled: true
		onClicked: pick => {
			root.isLight = !root.isLight;
			console.log(pick.entity, pick.distance);
		}
		onMoved: pick => {
			console.log("Moved", pick.entity, pick.distance);
					 tr.translation = Qt.vector3d(pick.worldIntersection.x,
												  pick.worldIntersection.y,
												  0)
		}
	}

	components: [tr, picker]

	Entity {
		PhongMaterial {
			id: mat
			ambient: root.isLight ? "darkgreen" : "lightblue"
			specular: ambient
		}

		SphereMesh {
			id: mesh
			radius: 0.3
		}

		components: [mat, mesh]
	}

	Text2DEntity {
		id: label
		width: 20
		height: 5
		color: "green"
		font: Qt.font({
			family: "Consolas",
			pointSize: 4,
			bold: true
		})

		Transform {
			id: labelTr
			scale: 0.2
			translation: Qt.vector3d(0.3, 0.3, 0)
		}

		components: [labelTr]
	}
}
