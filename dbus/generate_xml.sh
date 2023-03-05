#!/bin/sh

BASEDIR=$(dirname "$0")

qdbuscpp2xml ${BASEDIR}/display.h -o ${BASEDIR}/org.aos.display.xml
qdbuscpp2xml ${BASEDIR}/hands.h -o ${BASEDIR}/org.aos.hands.xml
