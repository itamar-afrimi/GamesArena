import React, { useState, useEffect } from "react";
import { useNavigate } from "react-router-dom";
import "./compoments/Lobby.css";

// You can expand this mapping as you add more games!
const ICONS = {
  Chess: "♟️",
  "Tic-Tac-Toe": "❌",
  Sudoku: "🔢",
  Memory: "🧠",
  "2048": "🟪",
};

const GameSelection = () => {
  const [games, setGames] = useState([]);
  const [selectedGame, setSelectedGame] = useState("");
  const navigate = useNavigate();

  useEffect(() => {
    fetch(`${import.meta.env.VITE_API_URL}/api/games`)
      .then((response) => response.json())
      .then((data) => {
        if (data && data.games) setGames(data.games);
      })
      .catch((error) => console.error("Error fetching games:", error));
  }, []);

  const handleGameSelect = (game) => {
    setSelectedGame(game);
  };

  const handleGameStart = () => {
    if (!selectedGame) {
      alert("Please select a game!");
      return;
    }
    navigate("/battle_lobby", { state: { gameType: selectedGame } });
  };

  return (
    <div>
      <div className="lobby-games-title">Choose a Game</div>
      <div className="lobby-games-list">
        {games.length === 0 ? (
          <span style={{ color: "#888", fontStyle: "italic" }}>Loading games...</span>
        ) : (
          games.map((game) => (
            <button
              key={game}
              className={`lobby-game-btn${selectedGame === game ? " selected" : ""}`}
              onClick={() => handleGameSelect(game)}
              type="button"
              style={{
                border: selectedGame === game ? "2.5px solid #38bdf8" : "none",
                boxShadow: selectedGame === game ? "0 4px 16px #38bdf844" : undefined,
              }}
            >
              <span className="lobby-game-icon">{ICONS[game] || "🎲"}</span>
              {game}
            </button>
          ))
        )}
      </div>
      <button
        className="lobby-game-btn"
        style={{
          marginTop: "1.2em",
          width: "100%",
          background: selectedGame
            ? "linear-gradient(90deg, #22d3ee 0%, #6366f1 100%)"
            : "linear-gradient(90deg, #cbd5e1 0%, #e0e7ef 100%)",
          color: selectedGame ? "#fff" : "#64748b",
          cursor: selectedGame ? "pointer" : "not-allowed",
          fontWeight: 700,
        }}
        onClick={handleGameStart}
        disabled={!selectedGame}
        type="button"
      >
        {selectedGame ? `Start ${selectedGame}` : "Start Game"}
      </button>
    </div>
  );
};

export default GameSelection;
