import Qt3D.Core
import Qt3D.Extras
import Qt3D.Render
import Qt3D.Input

import PolydotTransformationUi

Entity {
	id: root

	property alias text: label.text
	property alias pos: tr.translation
	property bool isLight: false

	signal requestPosChange(vector3d newPos)

	Transform {
		id: tr
		scale: 0.5
	}

	ObjectPicker {
		id: picker
		dragEnabled: true
		onPressed: function(pick) {
			if (pick.button !== PickEvent.MiddleButton)
				return;
			root.isLight = true;
		}
		onReleased: function(pick) {
			if (pick.button !== PickEvent.MiddleButton)
				return;
			root.isLight = false;
		}
		onMoved: function(pick) {
			if (pick.buttons // Qt bug: pick.buttons is used instead of pick.button
					!== PickEvent.MiddleButton)
				return;
			requestPosChange(Qt.vector3d(pick.worldIntersection.x, pick.worldIntersection.y, 0))
		}
	}

	components: [tr, picker]

	Entity {
		PhongMaterial {
			id: mat
			ambient: root.isLight ? "lightblue" : "darkgreen"
			specular: ambient
		}

		SphereMesh {
			id: mesh
			radius: 0.3
			rings: 5
			slices: 5
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
