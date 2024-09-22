import Qt3D.Core

Entity {
	id: root

	property alias model: points.model

	NodeInstantiator {
		id: points

		//asynchronous: true

		delegate: PointWithName {
			text: name
			scale: pointScale
			textPointSize: namePointSize

			pos: position
			onRequestPosChange: newPos => position = newPos
		}
	}
}
