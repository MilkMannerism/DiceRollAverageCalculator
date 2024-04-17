# Dice Roll Statistics Calculator

## Introduction
The Dice Roll Statistics Calculator is a Windows application designed to track dice rolls for various players and dice types. It allows users to input dice rolls for different dice configurations (e.g., d2, d4, d6, d8, d10, d12, d20, d100), calculate statistics such as the average roll, most frequent rolls, latest roll, and compare individual averages against group averages excluding the player. This tool is especially useful for tabletop games and statistical analysis of dice roll outcomes.

## Features
- **Add and Remove Players**: Manage players dynamically during runtime.
- **Dice Roll Submission**: Input rolls for selected dice types and players.
- **Statistics Calculation**: Automatically calculates and updates statistics for each player.
- **History Review**: View and manage roll history for each player.
- **Data Persistence**: Save and load roll data to ensure progress is not lost.

## Installation
1. **Prerequisites**: Ensure that you have a Windows environment to run the executable.
2. **Downloading the Application**: Download the latest release from the GitHub Releases page.
3. **Running the Application**: Unzip the downloaded file and run the executable to start the application.

## Usage
### Adding Players
- Click the "Add Player" button to add a new player with default naming convention (Player 1, Player 2, etc.).
- Edit the player's name directly in the text box labeled with the player's default name.

### Inputting Rolls
- Select the desired dice type from the drop-down menu at the top of the application.
- Enter the roll result in the input box for the respective player and hit the "Submit" button to record the roll.

### Viewing and Managing History
- Click the "History" button next to each player to view all recorded rolls for the selected dice type.
- Remove specific rolls directly from the history dialog.

### Saving and Loading Data
- Data is automatically saved to `PlayerAndRollStatsSave.txt` in the application's directory.
- Use the "Save" menu option to manually save the current state.
- Upon launching the application, data will automatically load if a save file exists in the application directory.

### Exiting the Application
- Close the application through the window's close button or the "Exit" menu option.
- Upon exiting, the application will prompt to save if there are unsaved changes.

## Support
For support, please open an issue on the GitHub repository page. Include details such as the error messages, steps to reproduce the issue, and screenshots if applicable.

## Contributing
Contributions to the Dice Roll Statistics Calculator are welcome. Please fork the repository, make your changes, and submit a pull request for review.

## License
This project is licensed under the MIT License - see the LICENSE file for details.
