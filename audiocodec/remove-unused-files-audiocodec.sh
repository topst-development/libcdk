#!/bin/bash

rm -rf $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/audiocodec/include
rm -rf $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/audiocodec/share

rm -rf `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/audiocodec -name "*.a"`
rm -rf `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/audiocodec -name "*.la"`
rm -rf `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/audiocodec -name "*.pc"`

arm-none-linux-gnueabi-strip --strip-debug --strip-unneeded `find $LINUX_PLATFORM_ROOTDIR/$BUILDDIR/audiocodec/ -name "*.so*"`
