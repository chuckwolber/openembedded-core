SUMMARY = "Linux Bluetooth Stack Userland V5"
DESCRIPTION = "Linux Bluetooth stack V5 userland components.  These include a system configurations, daemons, tools and system libraries."
HOMEPAGE = "http://www.bluez.org"
SECTION = "libs"
LICENSE = "GPL-2.0-or-later & LGPL-2.1-or-later"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e \
                    file://COPYING.LIB;md5=fb504b67c50331fc78734fed90fb0e09 \
                    file://src/main.c;beginline=1;endline=24;md5=0ad83ca0dc37ab08af448777c581e7ac"
DEPENDS = "dbus glib-2.0"
PROVIDES += "bluez-hcidump"
RPROVIDES:${PN} += "bluez-hcidump"

PACKAGECONFIG ??= "obex-profiles \
    readline \
    ${@bb.utils.filter('DISTRO_FEATURES', 'systemd', d)} \
    a2dp-profiles \
    avrcp-profiles \
    bap-profiles \
    bass-profiles \
    mcp-profiles \
    ccp-profiles \
    vcp-profiles \
    micp-profiles \
    csip-profiles \
    asha-profiles \
    network-profiles \
    hid-profiles \
    hog-profiles \
    tools \
    deprecated \
    udev \
"
PACKAGECONFIG[obex-profiles] = "--enable-obex,--disable-obex,libical"
PACKAGECONFIG[readline] = "--enable-client,--disable-client,readline,"
PACKAGECONFIG[testing] = "--enable-testing,--disable-testing"
PACKAGECONFIG[midi] = "--enable-midi,--disable-midi,alsa-lib"
PACKAGECONFIG[systemd] = "--enable-systemd,--disable-systemd"
PACKAGECONFIG[cups] = "--enable-cups,--disable-cups,,cups"
PACKAGECONFIG[nfc] = "--enable-nfc,--disable-nfc"
PACKAGECONFIG[sap-profiles] = "--enable-sap,--disable-sap"
PACKAGECONFIG[a2dp-profiles] = "--enable-a2dp,--disable-a2dp"
PACKAGECONFIG[avrcp-profiles] = "--enable-avrcp,--disable-avrcp"
PACKAGECONFIG[network-profiles] = "--enable-network,--disable-network"
PACKAGECONFIG[hid-profiles] = "--enable-hid,--disable-hid"
PACKAGECONFIG[hog-profiles] = "--enable-hog,--disable-hog"
PACKAGECONFIG[health-profiles] = "--enable-health,--disable-health"
PACKAGECONFIG[bap-profiles] = "--enable-bap,--disable-bap"
PACKAGECONFIG[bass-profiles] = "--enable-bass,--disable-bass"
PACKAGECONFIG[mcp-profiles] = "--enable-mcp,--disable-mcp"
PACKAGECONFIG[ccp-profiles] = "--enable-ccp,--disable-ccp"
PACKAGECONFIG[vcp-profiles] = "--enable-vcp,--disable-vcp"
PACKAGECONFIG[micp-profiles] = "--enable-micp,--disable-micp"
PACKAGECONFIG[csip-profiles] = "--enable-csip,--disable-csip"
PACKAGECONFIG[asha-profiles] = "--enable-asha,--disable-asha"
PACKAGECONFIG[sixaxis] = "--enable-sixaxis,--disable-sixaxis"
PACKAGECONFIG[tools] = "--enable-tools,--disable-tools"
PACKAGECONFIG[threads] = "--enable-threads,--disable-threads"
PACKAGECONFIG[deprecated] = "--enable-deprecated,--disable-deprecated"
PACKAGECONFIG[mesh] = "--enable-mesh --enable-external-ell,--disable-mesh, json-c ell"
PACKAGECONFIG[btpclient] = "--enable-btpclient --enable-external-ell,--disable-btpclient, ell"
PACKAGECONFIG[udev] = "--enable-udev,--disable-udev,udev"
PACKAGECONFIG[manpages] = "--enable-manpages,--disable-manpages,python3-docutils-native"

SRC_URI = "${KERNELORG_MIRROR}/linux/bluetooth/bluez-${PV}.tar.xz \
           file://init \
           file://run-ptest \
           file://0001-tests-add-a-target-for-building-tests-without-runnin.patch \
           file://0001-bluez5-disable-aics-tests.patch \
           "
S = "${UNPACKDIR}/bluez-${PV}"

CVE_PRODUCT = "bluez"

inherit autotools pkgconfig systemd update-rc.d ptest gobject-introspection-data

EXTRA_OECONF = "\
  --enable-test \
  --enable-datafiles \
  --enable-library \
  --enable-pie  \
  --without-zsh-completion-dir \
"

CFLAGS += "-DFIRMWARE_DIR='"${nonarch_base_libdir}/firmware"'"

# bluez5 builds a large number of useful utilities but does not
# install them.  Specify which ones we want put into ${PN}-noinst-tools.
NOINST_TOOLS_READLINE ??= ""
NOINST_TOOLS_TESTING ??= ""
NOINST_TOOLS_BT ??= ""
NOINST_TOOLS = " \
    ${@bb.utils.contains('PACKAGECONFIG', 'readline', '${NOINST_TOOLS_READLINE}', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'testing', '${NOINST_TOOLS_TESTING}', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'tools', '${NOINST_TOOLS_BT}', '', d)} \
"

do_install:append() {
	install -d ${D}${INIT_D_DIR}
	install -m 0755 ${UNPACKDIR}/init ${D}${INIT_D_DIR}/bluetooth

	if [ -f ${D}${sysconfdir}/init.d/bluetooth ]; then
		sed -i -e 's#@LIBEXECDIR@#${libexecdir}#g' ${D}${sysconfdir}/init.d/bluetooth
	fi

	# Install desired tools that upstream leaves in build area
	for f in ${NOINST_TOOLS} ; do
		install -m 755 ${B}/$f ${D}${bindir}
	done
}

PACKAGES =+ "${PN}-testtools ${PN}-obex ${PN}-noinst-tools"

FILES:${PN} += " \
    ${libdir}/bluetooth/plugins/*.so \
    ${systemd_unitdir}/ ${datadir}/dbus-1 \
    ${libdir}/cups \
"
FILES:${PN}-dev += " \
    ${libdir}/bluetooth/plugins/*.la \
"

FILES:${PN}-obex = "${libexecdir}/bluetooth/obexd \
                    ${exec_prefix}/lib/systemd/user/obex.service \
                    ${systemd_system_unitdir}/obex.service \
                    ${sysconfdir}/systemd/system/multi-user.target.wants/obex.service \
                    ${datadir}/dbus-1/services/org.bluez.obex.service \
                    ${sysconfdir}/dbus-1/system.d/obexd.conf \
                   "
SYSTEMD_SERVICE:${PN}-obex = "obex.service"

FILES:${PN}-testtools = "${libdir}/bluez/test/*"

def get_noinst_tools_paths (d, bb, tools):
    s = list()
    bindir = d.getVar("bindir")
    for bdp in tools.split():
        f = os.path.basename(bdp)
        s.append("%s/%s" % (bindir, f))
    return "\n".join(s)

FILES:${PN}-noinst-tools = "${@get_noinst_tools_paths(d, bb, d.getVar('NOINST_TOOLS'))}"

RDEPENDS:${PN}-testtools += "python3-core python3-dbus"
RDEPENDS:${PN}-testtools += "${@bb.utils.contains('GI_DATA_ENABLED', 'True', 'python3-pygobject', '', d)}"

SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('PACKAGECONFIG', 'systemd', 'bluetooth.service', '', d)}"
INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME:${PN} = "bluetooth"

do_compile_ptest() {
	oe_runmake buildtests
}

do_install_ptest() {
	cp -r ${B}/unit/ ${D}${PTEST_PATH}
	rm -f ${D}${PTEST_PATH}/unit/*.o
}

RDEPENDS:${PN}-ptest:append:libc-glibc = " glibc-gconv-utf-16"
