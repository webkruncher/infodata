

Clone libdb:
git clone https://github.com/berkeleydb/libdb.git

Steps to build libdb:

Note: Build on OpenBSD fails, the following kludge can be executed to
build the library.  The kludged library has not been fully tested.

sed -i 's/\(__atomic_compare_exchange\)/\1_db/' src/dbinc/atomic.h

Build:


export CC=gcc
cd build_unix
../dist/configure \
                --prefix=/usr \
                --enable-cxx \
		--enable-static \
		--disable-shared

                #--enable-dbm \
                #--disable-static \
                #--enable-compat185 \
make





#sudo make docdir=/usr/share/doc/db-5.3.28 install 

sudo chown -R root                        \
      /usr/bin/db_*                          \
      /usr/include/db{,_185,_cxx}.h          \
      /usr/lib/libdb*.{so,la}                \
      /usr/share/doc/db-5.3.28

