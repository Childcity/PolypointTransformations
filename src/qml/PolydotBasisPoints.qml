import Qt3D.Core

Entity {
	id: root

	property alias model: points.model

	NodeInstantiator {
		id: points
		model: root.model

		asynchronous: true

		delegate: PointWithName {
			text: name
			pos: position
			onRequestPosChange: newPos => position = newPos
		}
	}
}
