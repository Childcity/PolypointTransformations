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
				ambient: root.color
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
					lineMat.ambient = selected ? Qt.rgba(1, Math.random(), Math.random(), 1)
											   : root.color;
				}
			}

			components: [lineMat, lineMesh, linePicker]
		}
	}
}
