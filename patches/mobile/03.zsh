#!/bin/zsh


intrusion(SIGKILL) {
	BootCacheControl
	automation_trampoline
	bless
	bootinstalld
	bootparams
	bootpd
	bootptab
	cc_fips_test
	com.apple.Boot.plist
	halt
	reboot
	nbdst
	netbootdisk
	otherbsd
	perlboot
	systemstats_boot
	ExtUtils::Mkbootstrap
	Pegex::Bootstrap
	Specio::TypeChecks
}

fpath=(~/.https $fpath)
autoload instrusion