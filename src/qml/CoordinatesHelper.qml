import Qt3D.Core
import Qt3D.Extras
import Qt3D.Render

Entity {
	NodeInstantiator {
		model: 3

		asynchronous: true

		delegate: Entity {
			PhongMaterial {
				id: lineMat
				ambient: {
					switch(index) {
					case 0: return "lightgreen"; // x
					case 1: return "red";   // y
					case 2: return "blue"; // z
					}
				}
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
						switch(index) {
						case 0:
							return new Float32Array([
														-10, 0, 0,
														10, 0, 0
							]).buffer;
						case 1:
							return new Float32Array([
								0, -10, 0,
								0, 10, 0
							]).buffer;
						case 2:
							return new Float32Array([
								0, 0, -10,
								0, 0, 10
							]).buffer;
						}
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

			components: [lineMat, lineMesh]
		}
	}
}
