dnl MT_ARG_ENABLE(option, helpmessage, default)
AC_DEFUN([MT_ARG_ENABLE],
	[
		AC_ARG_ENABLE($1, [$2],
			[
				if test "$enableval" = no; then
					[enable_]patsubst([$1], -, _)="no";
				else
					[enable_]patsubst([$1], -, _)="yes";
				fi
			],
			[
				[enable_]patsubst([$1], -, _)=ifelse([$3],, yes, [$3])
			]
		)
	])

dnl MT_ARG_WITH_DIR(option, helpmessage)
AC_DEFUN(MT_ARG_WITH_DIR,
	[
		AC_ARG_WITH($1, [$2],
			[
				if test "$withval" = yes; then
					[with_]patsubst([$1], -, _)=""
				fi
			])
	])

dnl AC_FIND_FILE(file, dirs, variable)
AC_DEFUN(AC_FIND_FILE,
	[
		$3=NO
		for x in $2; do
			for y in $1; do
				if test -r "$x/$y"; then
					$3=$x
					break 2
				fi
			done
		done
	])

dnl MT_SEARCH_INCLUDES_AND_LIBS(library, include-file, include-dirs, library-file, library-dirs)
AC_DEFUN([MT_SEARCH_INCLUDES_AND_LIBS],
	[
		$1[_found]=no
		search_path="[$ac_]$1_includes $3"
		AC_FIND_FILE($2, $search_path, $1[_includes])
		if test "[$]$1[_includes]" != "NO"; then
			search_path="[$ac_]$1_libraries $5"
			AC_FIND_FILE($4, $search_path, $1[_libraries])
			if test "[$]$1[_libraries]" != "NO"; then
				$1[_found]=yes
			fi
		fi
	])

dnl MT_SUBST_CONFIG(variable)
AC_DEFUN([MT_SUBST_CONFIG],
	[
		if test "$enable_$1" = yes; then [USE_]translit([$1], a-z, A-Z)=1; else [USE_]translit([$1], a-z, A-Z)=0; fi
		AC_SUBST([USE_]translit([$1], a-z, A-Z))
	])

dnl MT_SUBST_SUBDIRS(variable, subdirs)
AC_DEFUN([MT_SUBST_SUBDIRS],
	[
		[$1]=""
		for i in [$2]; do
			eval "value=\$enable_$i"
			if test "x$value" = "xyes"; then
				[$1]="[$][$1] $i"
			fi
		done

		AC_SUBST([$1])
	])
