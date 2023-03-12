
import QtQuick 2.0
import org.asteroid.controls 1.0
import org.asteroid.utils 1.0
import org.asteroid.seconddisplay 1.0

Item {
    SecondDisplay {
        id: secondDisplay
    }

    Flickable {
        anchors.fill: parent
        contentHeight: onOffSettings.implicitHeight
        boundsBehavior: Flickable.DragOverBounds
        flickableDirection: Flickable.VerticalFlick
        anchors.margins: Dims.l(15)

        Column {
            id: onOffSettings
            anchors.fill: parent

            LabeledSwitch {
                text: "Stepcounter"
                visible: secondDisplay.hasStepCounter
                width: parent.width
                height: Dims.l(20)
                checked: secondDisplay.stepCounter
                onCheckedChanged: secondDisplay.stepCounter = checked
            }

            Item { width: parent.width; height: Dims.l(10) }
        }
    }
}
