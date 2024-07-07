import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore

Item {
	id: root

	property alias labelText: label.text

	property alias value: slider.value
	property alias minValue: slider.from
	property alias maxValue: slider.to
	property alias step: slider.stepSize

	property alias sliderWidth: slider.implicitWidth

	signal valueChangeRequested(real requestedValue)

	implicitWidth: content.implicitWidth
	implicitHeight: content.implicitHeight

	RowLayout {
		id: content

		spacing: 8

		Label {
			id: label

			Layout.preferredWidth: 80

			Layout.alignment: Qt.AlignLeft

			verticalAlignment: TextInput.AlignVCenter
			wrapMode: Text.WordWrap
		}

		Item {
			width: 0
			Layout.alignment: Qt.AlignRight
		}

		Slider {
			id: slider

			Layout.alignment: Qt.AlignRight

			implicitHeight: 24

			from: -1
			to: 1
			stepSize: 0.1

			onValueChanged: {
				valueChangeRequested(slider.value)
			}

			WheelHandler {
				acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad

				onWheel: event => {
					if (event.angleDelta.y > 0) {
						slider.increase();
					} else {
						slider.decrease();
					}
				}
			}
		}

		Item {
			id: numField

			Layout.alignment: Qt.AlignRight

			implicitWidth: 40
			implicitHeight: 24

			TextField {
				id: input

				anchors.fill: parent

				text: slider.value.toFixed(3)

				verticalAlignment: TextInput.AlignVCenter

				validator: DoubleValidator {
					bottom: root.minValue
					top: root.maxValue
					notation: DoubleValidator.StandardNotation
				}

				Keys.onEscapePressed: {
					focus = false;
				}

				Keys.onEnterPressed: {
					focus = false;
				}

				Keys.onReturnPressed: {
					focus = false;
				}

				onActiveFocusChanged: {
					if (activeFocus) {
						selectAll();
					} else {
						slider.value = text;
						root.valueChangeRequested(text);
					}
				}
			}
		}
	}

	Settings {
		location: "file:settings.ini"
		category: root.labelText

		property alias value: root.value
	}
}
