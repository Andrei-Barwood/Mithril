# Error Contract Matrix (Sprint 4)

This matrix documents the expected `mithril_status` for core error scenarios.
It is enforced by `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/tests/integration/test_error_contracts.c`.

| Scenario | Expected status |
|---|---|
| Tampered AEAD ciphertext/tag during decrypt | `MITHRIL_ERR_AEAD_AUTH_FAILED` |
| Invalid signature / wrong key / modified message | `MITHRIL_ERR_SIGNATURE_INVALID` |
| Operation not supported by active provider | `MITHRIL_ERR_NO_PROVIDER` |
| Invalid API argument (null pointers, bad lengths) | `MITHRIL_ERR_INVALID_ARGUMENT` |
| Unknown status returned by provider crypto op | `MITHRIL_ERR_INTERNAL` |
| Verification-like status in provider lifecycle (`on_activate`) | `MITHRIL_ERR_INTERNAL` |
| Unknown status in provider lifecycle (`on_activate`, `get_capabilities`) | `MITHRIL_ERR_INTERNAL` |

## Notes

- Verification failures are explicit only for cryptographic verify/auth operations.
- Provider lifecycle and registry paths never expose verification-specific statuses; they are normalized to internal errors.
