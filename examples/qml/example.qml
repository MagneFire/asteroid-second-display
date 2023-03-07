
import QtQuick 2.0
import org.asteroid.seconddisplay 1.0

Item {
    SecondDisplay {
        id: secondDisplay

        Component.onCompleted: {
            console.log("secondDisplay: " + secondDisplay.hasSecondDisplay)
        }
    }
    Hands {
        id: hands

        Component.onCompleted: {
            console.log("hands: " + hands.hasHands)
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: secondDisplay.synchronizeTime()
    }
}