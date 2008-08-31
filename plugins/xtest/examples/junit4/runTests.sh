#!/bin/bash

javac -cp junit.jar *.java
java -cp junit.jar:. org.junit.runner.JUnitCore MoneyTest
