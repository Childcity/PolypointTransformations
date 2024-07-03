import QtQuick
import QtQuick.Controls
import QtQuick.Scene3D

import PolydotTransformationUi

ApplicationWindow {
	id: root

	visible: true

	width: 1280
	height: 720

	Scene3D {
		anchors.fill: parent
		anchors.margins: 8

		aspects: ["input", "logic"]
		cameraAspectRatioMode: Scene3D.AutomaticAspectRatio

		SceneRoot {}
	}
}
