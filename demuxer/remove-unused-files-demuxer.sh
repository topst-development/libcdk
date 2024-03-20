#!/bin/bash

rm -rf $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/demuxer/include
rm -rf $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/demuxer/share

rm -rf `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/demuxer -name "*.a"`
rm -rf `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/demuxer -name "*.la"`
rm -rf `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/demuxer -name "*.pc"`

arm-none-linux-gnueabi-strip --strip-debug --strip-unneeded `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/demuxer/ -name "*.so*"`
