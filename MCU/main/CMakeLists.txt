idf_component_register(
	SRCS
		"main.c"
		"app_nvs.c"
		"app_wifi.c"
		"app_spiffs.c"
		"app_gitt.c"
		"app_time.c"

		# LibSSH
		"LibSSH-ESP32/src/agent.c"
		"LibSSH-ESP32/src/auth.c"
		"LibSSH-ESP32/src/base64.c"
		"LibSSH-ESP32/src/bignum.c"
		"LibSSH-ESP32/src/bind.c"
		"LibSSH-ESP32/src/bind_config.c"
		"LibSSH-ESP32/src/buffer.c"
		"LibSSH-ESP32/src/callbacks.c"
		"LibSSH-ESP32/src/channels.c"
		"LibSSH-ESP32/src/client.c"
		"LibSSH-ESP32/src/config.c"
		"LibSSH-ESP32/src/config_parser.c"
		"LibSSH-ESP32/src/connect.c"
		"LibSSH-ESP32/src/crypto_common.c"
		"LibSSH-ESP32/src/curve25519.c"
		"LibSSH-ESP32/src/dh.c"
		"LibSSH-ESP32/src/dh_key.c"
		"LibSSH-ESP32/src/dh-gex.c"
		"LibSSH-ESP32/src/ecdh.c"
		"LibSSH-ESP32/src/ecdh_mbedcrypto.c"
		"LibSSH-ESP32/src/error.c"
		"LibSSH-ESP32/src/getrandom_mbedcrypto.c"
		"LibSSH-ESP32/src/init.c"
		"LibSSH-ESP32/src/kdf.c"
		"LibSSH-ESP32/src/kex.c"
		"LibSSH-ESP32/src/knownhosts.c"
		"LibSSH-ESP32/src/libmbedcrypto.c"
		"LibSSH-ESP32/src/libssh_esp32_compat.c"
		"LibSSH-ESP32/src/log.c"
		"LibSSH-ESP32/src/match.c"
		"LibSSH-ESP32/src/mbedcrypto_missing.c"
		"LibSSH-ESP32/src/md_mbedcrypto.c"
		"LibSSH-ESP32/src/messages.c"
		"LibSSH-ESP32/src/misc.c"
		"LibSSH-ESP32/src/options.c"
		"LibSSH-ESP32/src/packet.c"
		"LibSSH-ESP32/src/packet_cb.c"
		"LibSSH-ESP32/src/packet_crypt.c"
		"LibSSH-ESP32/src/pki.c"
		"LibSSH-ESP32/src/pki_container_openssh.c"
		"LibSSH-ESP32/src/pki_ed25519.c"
		"LibSSH-ESP32/src/pki_ed25519_common.c"
		"LibSSH-ESP32/src/pki_mbedcrypto.c"
		"LibSSH-ESP32/src/poll.c"
		"LibSSH-ESP32/src/scp.c"
		"LibSSH-ESP32/src/server.c"
		"LibSSH-ESP32/src/session.c"
		"LibSSH-ESP32/src/socket.c"
		"LibSSH-ESP32/src/string.c"
		"LibSSH-ESP32/src/threads.c"
		"LibSSH-ESP32/src/token.c"
		"LibSSH-ESP32/src/wrapper.c"
		"LibSSH-ESP32/src/threads/noop.c"
		"LibSSH-ESP32/src/threads/mbedtls.c"
		"LibSSH-ESP32/src/external/bcrypt_pbkdf.c"
		"LibSSH-ESP32/src/external/blowfish.c"

		# git_things
		"git_things/examples/gitt_log_impl.c"
		"git_things/examples/gitt_ssh_impl.c"
		"git_things/src/gitt_ssh.c"
		"git_things/src/gitt_sha1.c"
		"git_things/src/gitt_unpack.c"
		"git_things/src/gitt_misc.c"
		"git_things/src/gitt_zlib.c"
		"git_things/src/gitt_command.c"
		"git_things/src/gitt_repertory.c"
		"git_things/src/gitt_commit.c"
		"git_things/src/gitt_pack.c"
		"git_things/src/gitt.c"
		"git_things/third_party/zlib/adler32.c"
		"git_things/third_party/zlib/crc32.c"
		"git_things/third_party/zlib/deflate.c"
		"git_things/third_party/zlib/inffast.c"
		"git_things/third_party/zlib/inflate.c"
		"git_things/third_party/zlib/inftrees.c"
		"git_things/third_party/zlib/trees.c"
		"git_things/third_party/zlib/zutil.c"

        INCLUDE_DIRS
		"."

		# LibSSH
		"LibSSH-ESP32/src"
		"LibSSH-ESP32/src/threads"
		"LibSSH-ESP32/src/external"

		# git_things
		"git_things/include"
		"git_things/third_party/zlib"
)

add_definitions(
	# LibSSH
	-DESP32
	# -include freertos/FreeRTOS.h
	# -include freertos/task.h
	-Wno-int-conversion
	-Wno-discarded-qualifiers
	-Wno-deprecated-declarations
	-Wno-implicit-fallthrough
	# -Wno-macro-redefinition

	# zlib
	-DMAX_MEM_LEVEL=4
)
