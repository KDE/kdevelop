import QtQuick.tooling 1.1

// This file describes the plugin-supplied types contained in the library.
// It is used for QML tooling purposes only.
//
// This file was auto-generated with the command 'qml1plugindump Qt.labs.particles 1.0'.

Module {
    Component {
        name: "QDeclarativeParticleMotion"
        prototype: "QObject"
        exports: ["ParticleMotion 1.0"]
    }
    Component {
        name: "QDeclarativeParticleMotionGravity"
        prototype: "QDeclarativeParticleMotion"
        exports: ["ParticleMotionGravity 1.0"]
        Property { name: "xattractor"; type: "double" }
        Property { name: "yattractor"; type: "double" }
        Property { name: "acceleration"; type: "double" }
    }
    Component {
        name: "QDeclarativeParticleMotionLinear"
        prototype: "QDeclarativeParticleMotion"
        exports: ["ParticleMotionLinear 1.0"]
    }
    Component {
        name: "QDeclarativeParticleMotionWander"
        prototype: "QDeclarativeParticleMotion"
        exports: ["ParticleMotionWander 1.0"]
        Property { name: "xvariance"; type: "double" }
        Property { name: "yvariance"; type: "double" }
        Property { name: "pace"; type: "double" }
    }
    Component {
        name: "QDeclarativeParticles"
        defaultProperty: "motion"
        prototype: "QDeclarativeItem"
        exports: ["Particles 1.0"]
        Property { name: "source"; type: "QUrl" }
        Property { name: "count"; type: "int" }
        Property { name: "emissionRate"; type: "int" }
        Property { name: "emissionVariance"; type: "double" }
        Property { name: "lifeSpan"; type: "int" }
        Property { name: "lifeSpanDeviation"; type: "int" }
        Property { name: "fadeInDuration"; type: "int" }
        Property { name: "fadeOutDuration"; type: "int" }
        Property { name: "angle"; type: "double" }
        Property { name: "angleDeviation"; type: "double" }
        Property { name: "velocity"; type: "double" }
        Property { name: "velocityDeviation"; type: "double" }
        Property { name: "motion"; type: "QDeclarativeParticleMotion"; isPointer: true }
        Signal { name: "emittingChanged" }
        Method {
            name: "burst"
            Parameter { name: "count"; type: "int" }
            Parameter { name: "emissionRate"; type: "int" }
        }
        Method {
            name: "burst"
            Parameter { name: "count"; type: "int" }
        }
    }
}
