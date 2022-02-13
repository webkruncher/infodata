 

#. ../../krunchercore/Builder.ksh
#. ../../testfactory/utilities/BuildTools.ksh
#BuildAll &&  kruncher -v "SERVICE|DBCURSOR3|REST3" && ./go -krbuildertest

pushd ../../testfactory/db/src
Build -install
popd
Build -install
kruncher -v "SERVICE|DBCURSOR3|REST3" && ./go -krbuildertest

