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
		position: Qt.vector3d(0, 0, 3)
		upVector: Qt.vector3d(0, 1, 0)
		viewCenter: Qt.vector3d(0, 0, 0)
	}

	OrbitCameraController {
		camera: camera
		linearSpeed: 300
		lookSpeed: 400
	}

	components: [
		RenderSettings {
			activeFrameGraph: ForwardRenderer {
				clearColor: "#2d2d2d"
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

	CoordinatesHelper {}

	PolydotMeshList {
		model: mainController.meshListModel
		color: "yellow"
	}

	Connections {
		target: Application

		function onStateChanged() {
			if (Application.state === Qt.ApplicationActive) {
				mainController.loadMeshes();
			}
		}
	}
}
