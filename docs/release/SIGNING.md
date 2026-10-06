# Candidate signature verification

Dedicated public-key fingerprint:

    SHA256:xPd5gIdHhdx+W6u9YBiNEa/toMquLCAw9ci+GlmOBLY

The private key remains on the owner's Pi with mode 0600, outside this
repository. Only the public key and allowed-signers file are included.
No UDNOS or runtime authorization key is reused.

From the repository root, verify a signed commit/tag:

    git -c gpg.ssh.allowedSignersFile=docs/release/allowed-signers verify-commit HEAD
    git -c gpg.ssh.allowedSignersFile=docs/release/allowed-signers verify-tag qbit-nova-language-native-v0.1.1-rc.1

For the release directory containing SHA256SUMS and SHA256SUMS.sig:

    ssh-keygen -Y verify -f /path/to/repository/docs/release/allowed-signers -I qbit-nova-release -n qbit-nova-release -s SHA256SUMS.sig < SHA256SUMS
    sha256sum -c SHA256SUMS

Validate the fingerprint through a trusted channel before trusting a
downloaded key. A key bundled with an archive is not independent proof of
the publisher's identity. Local signature verification is not a claim of
GitHub account verification or owner approval of a stable release.
