#!/bin/bash

javac -cp junit.jar *.java
java -cp junit.jar:. junit.textui.TestRunner MoneyTest
