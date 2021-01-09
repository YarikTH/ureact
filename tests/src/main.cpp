#define APPROVALS_DOCTEST // This tells Approval Tests to provide a main() - only do this in one cpp file
#include "ApprovalTests.hpp"

using namespace ApprovalTests;

auto directoryDisposer = Approvals::useApprovalsSubdirectory("approval_tests");
