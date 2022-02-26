 

#. ../../krunchercore/Builder.ksh
#. ../../testfactory/utilities/BuildTools.ksh
#BuildAll &&  kruncher -v "SERVICE|DBCURSOR3|REST3" && ./go -krbuildertest

#pushd ../../testfactory/utilities/src
#Build -install
#popd
#
pushd ../../informationkruncher/src
Build -install
popd
#
pushd ../../testfactory/src
Build -install
popd

pushd ../../testfactory/db/src
Build -install
popd

Build -clean -install
kruncher $@ -v "SERVICE|DBCURSOR3|REST3" 

#&& ./go -krbuildertest

