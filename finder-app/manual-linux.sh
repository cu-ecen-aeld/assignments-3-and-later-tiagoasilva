#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
CURRENTDIR=$(pwd)
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
CFLAGS="-march=armv8-a -O2 -g -Wall -static"

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

mkdir -p ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone https://git.busybox.net/busybox/
    cd busybox
    git checkout ${BUSYBOX_VERSION}

    make distclean
    make defconfig
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
else
    cd busybox
fi

make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"

cp $(${CROSS_COMPILE}gcc -print-file-name=ld-linux-aarch64.so.1) ${OUTDIR}/rootfs/lib/
cp $(${CROSS_COMPILE}gcc -print-file-name=libm.so.6) ${OUTDIR}/rootfs/lib64/
cp $(${CROSS_COMPILE}gcc -print-file-name=libresolv.so.2) ${OUTDIR}/rootfs/lib64/
cp $(${CROSS_COMPILE}gcc -print-file-name=libc.so.6) ${OUTDIR}/rootfs/lib64/

if [ -f "${OUTDIR}/rootfs/dev/null" ]
then
    sudo rm ${OUTDIR}/rootfs/dev/null
fi
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
if [ -f "${OUTDIR}/rootfs/dev/tty" ]
then
    sudo rm ${OUTDIR}/rootfs/dev/tty
fi
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/tty c 5 1

cd "$CURRENTDIR"
make clean
make CROSS_COMPILE=${CROSS_COMPILE} CFLAGS="${CFLAGS}"

cp autorun-qemu.sh finder-test.sh finder.sh writer ${OUTDIR}/rootfs/home/
cp -L -r conf ${OUTDIR}/rootfs/home/conf/

cd "${OUTDIR}/rootfs"
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ..
gzip -f initramfs.cpio
