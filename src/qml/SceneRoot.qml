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
		position: Qt.vector3d(0, 0, 20)
		upVector: Qt.vector3d(0, 1, 0)
		viewCenter: Qt.vector3d(0, 0, 0)
	}

	OrbitCameraController {
		camera: camera
		linearSpeed: 200
	}

	components: [
		RenderSettings {
			activeFrameGraph: ForwardRenderer {
				clearColor: "#2d2d2d"
				camera: camera
			}
			pickingSettings.pickMethod: PickingSettings.LinePicking
		},
		InputSettings {}
	]

	MainController{
		id: mainController
	}

	Entity {
		PhongMaterial {
			id: pixMat
			ambient: "red"
		}

		Transform {
			id: pixTr
			rotationX: 90
		}

		PlaneMesh {
			id: pixMesh
			width: 1
			height: 1
			meshResolution: Qt.size(2, 2)
		}

		components: [pixMat, pixTr, pixMesh]
	}

	PolydotMeshList {
		model: mainController.meshListModel
		color: "lightgreen"
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
