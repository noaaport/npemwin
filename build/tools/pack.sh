#!/bin/sh
#
# pack the working directory for building in other machines
#
name=npemwin

cd ../../..
tar czf ~/${name}.tgz --exclude=${name}/dev-notes ${name}
