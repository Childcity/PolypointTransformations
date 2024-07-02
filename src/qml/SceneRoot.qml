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
		position: Qt.vector3d(0, 0, 90)
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
			pickingSettings.pickMethod: PickingSettings.LinePicking | PickingSettings.PointPicking
		},
		InputSettings {}
	]

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
			width: 1.0
			height: 1.0
			meshResolution: Qt.size(2, 2)
		}

		components: [pixMat, pixTr, pixMesh]
	}

	// Entity {
	// 	PhongMaterial {
	// 		id: importedMat
	// 		ambient: "green"
	// 	}

	// 	Transform {
	// 		id: importedTr
	// 		//rotationX: 180
	// 		scale: 50
	// 	}

	// 	LineGeometry {
	// 	}

	// 	//SceneLoader {
	// 	//	id: importedScene
	// 	//	source: "file:///C:/Users/Ariel/Documents/AAScetch/exp/Untitled.dae"
	// 	//}

	// 	components: [importedMat, importedTr, importedScene]
	// }
	// QSG_RHI_BACKEND=opengl
	// QSG_RHI_PREFER_SOFTWARE_RENDERER=0

	Entity {
		objectName: "LINE 1"

		PhongMaterial {
			id: lineMat
			ambient: "lightgreen"
		}

		Transform {
			id: lineTr
			//rotationX: 180
			scale: 10
		}

		Attribute {
			id: positionAttribute
			name: defaultPositionAttributeName
			attributeType: Attribute.VertexAttribute
			vertexBaseType: Attribute.Float
			vertexSize: 3
			byteStride: 3 * 4 // Float
			byteOffset: 0
			buffer: Buffer {
				data: {
					const arr = new Float32Array([
													 // -5, 0,  0,
													 // 5, 0,  0,
													 -5, -1,  1,
													 5, -1,  1,
													 5,  1,  1,
													-5,  1,  1,
													-5, -1, -1,
													 5, -1, -1,
													 5,  1, -1,
													-5,  1, -1
					]);
					return arr.buffer;
				}
			}
			count: 8
		}

		Attribute {
			id: indiexAttribute
			attributeType: Attribute.IndexAttribute
			vertexBaseType: Attribute.UnsignedInt
			vertexSize: 1
			byteStride: 1 * 4 // UnsignedInt
			byteOffset: 0
			buffer: Buffer {
				data: {
					const arr = new Uint32Array([
													 0, 1, 1, 2, 2, 3, 3, 0,  // Front face
													 4, 5, 5, 6, 6, 7, 7, 4,  // Back face
													 0, 4, 1, 5, 2, 6, 3, 7   // Connecting edges
					]);
					return arr.buffer;
				}
			}
			count: 24
		}

		Attribute {
			id: boundingVolumeAttribute
			attributeType: Attribute.VertexAttribute
			vertexBaseType: Attribute.Float
			vertexSize: 3
			byteStride: 3 * 4 // Float
			byteOffset: 0
			buffer: Buffer {
				data: {
					const arr = new Float32Array([
													 -5, -0.1,  0.1,
													 5, -0.1,  0.1,
													 5,  0.1,  0.1,
													-5,  0.1,  0.1,
													-5, -0.1, -0.1,
													 5, -0.1, -0.1,
													 5,  0.1, -0.1,
													-5,  0.1, -0.1
					]);
					return arr.buffer;
				}
			}
			count: 8
		}

		GeometryRenderer {
			id: lineMesh
			primitiveType: GeometryRenderer.Lines
			geometry: Geometry {
				boundingVolumePositionAttribute: boundingVolumeAttribute
				attributes: [
					positionAttribute,
					indiexAttribute
				]
			}
		}

		ObjectPicker {
			id: linePicker
			onClicked: event => {
			lineMat.ambient =  Qt.rgba(255, Math.random(), Math.random(), 255)
				console.log(event,event.entity,event.distance, event.triangleIndex)
			}
		}

		MainController{
			id: main
		}

		components: [lineMat, lineTr, lineMesh, linePicker]
	}

	//Text2DEntity {
	//	id: text
	//	width: 400
	//	height: 10
	//	text: "Hello World"
	//	color: "green"
	//	font: Qt.font({
	//		family: "Consolas",
	//		pointSize: 8,
	//		bold: true
	//	})
	//}
}
