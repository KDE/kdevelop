#!/bin/bash

gmcs -target:library -r:nunit.core.dll -r:nunit.framework.dll moneytest.cs money.cs
nunit-console2 moneytest.dll
