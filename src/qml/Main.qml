import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import PolydotTransformationUi

ApplicationWindow {
	id: root

	visible: true

	width: 1280
	height: 720


	MainController {
		id: controller
	}

	Connections {
        target: Application

        function onStateChanged() {
			if (Application.state === Qt.ApplicationActive) {
			}
        }
    }

	Settings {
		location: "file:settings.ini"
		category: "Main"
	}

	QtObject {
		id: internal
	}
}
