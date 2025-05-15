import React, { useState, useEffect } from "react";
import { useNavigate } from 'react-router-dom';


const GameSelection = () => {
  const [games, setGames] = useState([]);
  const [selectedGame, setSelectedGame] = useState("");

    const navigate = useNavigate(); // Use useNavigate from react-router-dom
  // Fetch available games from the backend
  useEffect(() => {
    fetch("http://localhost:18080/api/games")
      .then((response) => response.json())
      .then((data) => {
        if (data && data.games) {
          setGames(data.games);  // Update the games list
        } else {
          console.error("Failed to fetch games.");
        }
      })
      .catch((error) => console.error("Error fetching games:", error));
  }, []);

  // Handle game selection
  const handleGameSelect = (game) => {
    setSelectedGame(game);
  };

//   // Handle game start request
//   const handleStartGame = () => {
//     if (!selectedGame) {
//       alert("Please select a game!");
//       return;
//     }

//     fetch("http://localhost:18080/api/start_game", {
//       method: "POST",
//       headers: {
//         "Content-Type": "application/json",
//       },
//       body: JSON.stringify({
//         game: selectedGame,
//         username: username, // Add username from state
//       }),
//     })  
//       .then((response) => response.json())
//       .then((data) => {
//         console.log("Game started:", data);
//         navigate("/tictak"); // Navigate to the game screen
//         // Handle the response (e.g., navigate to the game screen or show waiting for another player)
//       })
//       .catch((error) => console.error("Error starting game:", error));
//   };
  const handleGameSelectRender = () => {
    navigate("/battle_lobby");
  };

  return (
    <div style={styles.container}>
      <h2>Select a Game</h2>
      {games.length > 0 ? (
        <ul>
          {games.map((game, index) => (
            <li
              key={index}
              onClick={() => handleGameSelect(game)}
              style={{
                cursor: "pointer",
                padding: "5px",
                background: selectedGame === game ? "lightgray" : "",
              }}
            >
              {game}
            </li>
          ))}
        </ul>
      ) : (
        <p>Loading games...</p>  // Show a loading message if no games are available yet
      )}
      <button onClick={handleGameSelectRender}>Start Game</button>
    </div>
  );
};

const styles = {
  container: {
    textAlign: "center",
    marginTop: "20px",
  },
};

export default GameSelection;
