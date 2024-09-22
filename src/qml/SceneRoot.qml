import QtQuick
import Qt3D.Core
import Qt3D.Extras
import Qt3D.Input
import Qt3D.Render

import PolydotTransformationUi

Entity {
	Camera {
		id: camera
		projectionType: CameraLens.PerspectiveProjection
		fieldOfView: 60
		nearPlane: 0.1
		farPlane: 1000.0
		position: Qt.vector3d(0, 0, 10)
		upVector: Qt.vector3d(0, 1, 0)
		viewCenter: Qt.vector3d(0, 0, 0)
	}

	OrbitCameraController {
		camera: camera
		linearSpeed: 50
		lookSpeed: 300
	}

	components: [
		RenderSettings {
			activeFrameGraph: ForwardRenderer {
				//clearColor: "#2d2d2d"
				clearColor: "white"
				camera: camera
			}
			pickingSettings.pickMethod: PickingSettings.PrimitivePicking
		},
		InputSettings {}
	]

	MainController{
		id: mainController
	}

	PolydotBasisPoints {
		model: mainController.basisPointsModel
	}

	//CoordinatesHelper {}

	PolydotMeshList {
		model: mainController.meshListModel
		//color: "yellow"
		color: "black"
	}

	Connections {
		target: Application

		function onStateChanged() {
			if (Application.state === Qt.ApplicationActive) {
				//mainController.loadMeshes();
			}
		}
	}

	Entity {
		PointLight {
			id: light
			constantAttenuation: 0.8
			//linearAttenuation: 0.001
			//quadraticAttenuation: 0.8

			//NumberAnimation on constantAttenuation {
			//	duration: 5000
			//	from: 0.1
			//	to: 0.6
			//}
			//onConstantAttenuationChanged: console.log("constantAttenuation", constantAttenuation)
		}
		Transform {
			id: lightTr
			translation: Qt.vector3d(0, 0, 5)
		}
		components: [light, lightTr]
	}
}
