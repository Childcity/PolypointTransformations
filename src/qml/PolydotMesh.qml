import Qt3D.Core
import Qt3D.Extras
import Qt3D.Render

Entity {
	id: root

	property alias model: mesh.model
	property color color: "black"

	NodeInstantiator {
		id: mesh

		asynchronous: true

		delegate: Entity {
			required property int index

			required property var model
			required property var lineGeometry
			required property bool selected

			objectName: "LINE " + index

			PhongMaterial {
				id: lineMat
				//ambient: selected ? Qt.rgba(1, Math.random(), Math.random(), 1) : root.color
				//ambient: selected ? "purple" : root.color
				ambient: selected ? "yellow" : root.color
				//ambient: selected ? "black" : root.color
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
							//0,0,0,
							//1,0,index
							lineGeometry.p1.x, lineGeometry.p1.y, 0,
							lineGeometry.p2.x, lineGeometry.p2.y, 0
						]);
						return arr.buffer;
					}
				}
				count: 2
			}

			GeometryRenderer {
				id: lineMesh
				primitiveType: GeometryRenderer.Lines
				geometry: Geometry {
					attributes: [
						positionAttribute
					]
				}
			}

			ObjectPicker {
				id: linePicker
				onClicked: pick => {
					model.selected = !selected;
				}
			}

			components: [lineMat, lineMesh, linePicker]

			// Text2DEntity {
			// 	id: label
			// 	width: 20
			// 	height: 5
			// 	color: "purple"
			// 	font: Qt.font({
			// 		family: "Consolas",
			// 		pointSize: 3,
			// 		bold: true
			// 	})

			// 	text: index + "p1"

			// 	Transform {
			// 		id: labelTr
			// 		scale: 0.06
			// 		translation: Qt.vector3d(lineGeometry.p1.x + Math.random()/3, lineGeometry.p1.y, 0)
			// 	}

			// 	components: [labelTr]
			// }

			// Text2DEntity {
			// 	id: label2
			// 	width: 20
			// 	height: 5
			// 	color: "gray"
			// 	font: Qt.font({
			// 		family: "Consolas",
			// 		pointSize: 3,
			// 		bold: true
			// 	})

			// 	text: index + "p2"

			// 	Transform {
			// 		id: label2Tr
			// 		scale: 0.06
			// 		translation: Qt.vector3d(lineGeometry.p2.x, lineGeometry.p2.y + Math.random()/3, 0)
			// 	}

			// 	components: [label2Tr]
			// }
		}
	}
}
