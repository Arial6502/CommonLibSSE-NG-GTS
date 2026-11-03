// Loads the real SE 1.5.97, AE 1.6.1170 and AE 1.7.104 address libraries through
// REL::IDDatabase and checks ids against addresses read in Ghidra.
//
//   test_addresslib <dir with version-1-5-97-0.bin, versionlib-1-6-1170-0.bin, versionlib-1-7-104-0.bin>
#define ENABLE_COMMONLIBSSE_TESTING
#include "SKSE/Impl/PCH.h"
#include "SKSE/Version.h"

#include <cstdio>

namespace
{
	int failures = 0;

	void expect(bool a_ok, const char* a_what)
	{
		std::printf("%s %s\n", a_ok ? "PASS" : "FAIL", a_what);
		failures += !a_ok;
	}

	bool load(const std::filesystem::path& a_dir, const wchar_t* a_file, REL::Version a_version, REL::IDDatabase::Format a_format)
	{
		REL::Module::mock(a_version);
		return REL::IDDatabase::inject((a_dir / a_file).wstring(), a_format, a_version);
	}
}

int wmain(int a_argc, wchar_t** a_argv)
{
	if (a_argc != 2) {
		return 2;
	}
	const std::filesystem::path dir = a_argv[1];
	const REL::Version se(1, 5, 97, 0), ae16(1, 6, 1170, 0), ae17(1, 7, 104, 0);

	expect(load(dir, L"version-1-5-97-0.bin", se, REL::IDDatabase::Format::SSEv1), "SE 1.5.97 format 1 loads");
	expect(REL::Module::IsSE(), "1.5.97 classifies as SE");
	expect(REL::IDDatabase::get().id2offset(13212) == 0x157360, "SE 13212 is TES::GetWaterHeight at 0x157360");

	expect(load(dir, L"versionlib-1-6-1170-0.bin", ae16, REL::IDDatabase::Format::SSEv2), "AE 1.6.1170 format 2 loads");
	expect(REL::Module::IsAE(), "1.6.1170 classifies as AE");
	expect(REL::IDDatabase::get().id2offset(13358) == 0x1A0D20, "1170 13358 is TES::GetWaterHeight at 0x1A0D20");
	expect(REL::IDDatabase::get().id2offset(441582) == 0x33D6A0, "1170 441582 is Script::CompileAndRun_Impl at 0x33D6A0");

	expect(load(dir, L"versionlib-1-7-104-0.bin", ae17, REL::IDDatabase::Format::SSEv2), "AE 1.7.104 format 5 loads through the AE format 2 path");
	expect(REL::Module::IsAE(), "1.7.104 classifies as AE");
	expect(REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99), "1.7.104 is at least 1.7.99");
	expect(REL::IDDatabase::get().id2offset(523463) == 0x1A6130, "1.7.104 523463 is TES::GetWaterHeight at 0x1A6130");
	expect(REL::IDDatabase::get().id2offset(441582) == 0x343D00, "1.7.104 441582 is Script::CompileAndRun_Impl at 0x343D00");
	expect(AE_CHECK(SKSE::RUNTIME_SSE_1_7_99, 13358, 523463) == 523463, "AE_CHECK picks the 1.7 id on 1.7.104");

	expect(!load(dir, L"versionlib-1-7-104-0.bin", ae16, REL::IDDatabase::Format::SSEv2), "format 5 with the wrong version is rejected");
	expect(!load(dir, L"versionlib-1-7-104-0.bin", se, REL::IDDatabase::Format::SSEv1), "format 5 under an SE expectation is rejected");

	REL::Module::mock(ae16);
	expect(!REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99), "1.6.1170 is not at least 1.7.99");
	expect(AE_CHECK(SKSE::RUNTIME_SSE_1_7_99, 13358, 523463) == 13358, "AE_CHECK picks the 1.6 id on 1.6.1170");

	std::printf("%d failure(s)\n", failures);
	return failures ? 1 : 0;
}
