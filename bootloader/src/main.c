#include <c-efi.h>

CEfiStatus efi_main(CEfiHandle h, CEfiSystemTable *st) {
    CEfiStatus r = st->con_out->output_string(st->con_out, L"Ik heb aids\n");
    if (C_EFI_ERROR(r)) {
        return r;
    }

    CEfiUSize x;
    r = st->boot_services->wait_for_event(1, &st->con_in->wait_for_key, &x);
    if (C_EFI_ERROR(r)) {
        return r;
    }

    return C_EFI_SUCCESS;
}