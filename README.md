# README

## To run the project (Windows System)
1. Extract the contents of the zip folder (to wherever you'd like)
2. Open the ``.sln`` file (Visual Studio Solution file)
3. Click on the ``Local Windows Debugger`` button to run

## To alternate between CMultiThread and DeadlockQuestion
By default, CMultiThread will run. To test DeadlockQuestion,
comment out the entirety of the ``int main...`` function in ``main.cpp``
and uncomment the ``int main...`` function in ``DeadlockQuestion.cpp``.

The answer for why DeadlockQuestion is deadlocking is contained
in a comment in ``DeadlockQuestion.cpp``.
