import React, { useState, useEffect, useContext } from 'react';
import { useLocation, useNavigate } from 'react-router-dom';
import { UserContext } from '../UserContext';
import './Backgammon.css';

const POINTS = 24;
const BOARD_ROWS = 2;
const CHECKER_COLORS = { white: '#f7f7f7', black: '#222' };

const Backgammon = () => {
  const { state } = useLocation();
  const { sessionId } = state || {};
  const { username } = useContext(UserContext);
  const navigate = useNavigate();

  const [board, setBoard] = useState(Array.from({length: POINTS}, () => ({ color: null, count: 0 })));
  const [players, setPlayers] = useState([]);
  const [currentPlayer, setCurrentPlayer] = useState(null);
  const [dice, setDice] = useState([1, 2]);
  const [isFinished, setIsFinished] = useState(false);
  const [winner, setWinner] = useState(null);
  const [ws, setWs] = useState(null);
  const [waiting, setWaiting] = useState(false);
  const [selectedFrom, setSelectedFrom] = useState(null);


  // ====== 1. WebSocket setup ======
  useEffect(() => {
    if (!sessionId || !username) return;
    let backendHost = import.meta.env.VITE_API_URL || 'localhost:8080';
    backendHost = backendHost.replace(/^https?:\/\//, '');
    const protocol = window.location.protocol === 'https:' ? 'wss' : 'ws';
    const socket = new WebSocket(`${protocol}://${backendHost}/ws/game/${sessionId}`);

    socket.onopen = () => {
      socket.send(JSON.stringify({ sessionId, username }));
    };

    socket.onmessage = (event) => {
      const state = JSON.parse(event.data);
      setBoard(state.board);
      console.log("Received game state from backend:", state);
      setPlayers(state.players || []);
      setCurrentPlayer(state.currentPlayer);
      setDice(state.dice || [1, 2]);
      setWinner(state.winner || null);
      setIsFinished(state.finished || false);
      setWaiting(state.waiting || false);
    };

    socket.onclose = () => setWs(null);

    setWs(socket);
    return () => socket.close();
  }, [sessionId, username]);

  // ====== 2. Handle moves ======
  const handlePointClick = (pointIdx) => {
  if (!ws || ws.readyState !== 1 || isFinished || currentPlayer !== username) return;

  // If no source selected, select this point as FROM if it has player's checker
  if (selectedFrom === null) {
    const point = board[pointIdx];
    if (point.color === (username === players[0] ? 'black' : 'white') && point.count > 0) {
      setSelectedFrom(pointIdx);
    }
    return;
  }

  // If source already selected, this is the TO
  if (selectedFrom !== null) {
    // Send move to backend
    ws.send(JSON.stringify({ username, from: selectedFrom, to: pointIdx }));
    setSelectedFrom(null); // Reset for next move
  }
};


  // ====== 3. Render helpers ======
  const renderCheckers = (color, count) => {
    if (!color || count === 0) return null;
    return (
      <div className={`checkers ${color}`}>
        {Array.from({length: count}).map((_, i) => (
          <div className="checker" key={i} />
        ))}
      </div>
    );
  };

  // Split board into top and bottom rows for rendering
  const topPoints = board.slice(12, 24).reverse();
  const bottomPoints = board.slice(0, 12);

  // ====== 4. Render ======
  return (
    <div className="backgammon-container">
      <h1 className="bg-heading">🎲 Backgammon</h1>
      {waiting ? (
        <p className="bg-status">Waiting for another player to join...</p>
      ) : (
        <p className="bg-status">
          {winner
            ? `🏆 Winner: ${winner}`
            : currentPlayer === username
              ? 'Your turn!'
              : `${currentPlayer || '...'}'s turn`}
        </p>
      )}

      <div className="bg-board">
        <div className="bg-row bg-row-top">
          {topPoints.map((point, idx) => (
            <div
              className="bg-point"
              key={12 + (11 - idx)}
              onClick={() => handlePointClick(12 + (11 - idx))}
            >
              <div className="bg-point-num">{12 + (11 - idx) + 1}</div>
              {renderCheckers(point.color, point.count)}
            </div>
          ))}
        </div>
        <div className="bg-bar">
          <div className="bg-dice">
            <span className="bg-die">{dice[0]}</span>
            <span className="bg-die">{dice[1]}</span>
          </div>
        </div>
        <div className="bg-row bg-row-bottom">
          {bottomPoints.map((point, idx) => (
            <div
              className="bg-point"
              key={idx}
              onClick={() => handlePointClick(idx)}
            >
              <div className="bg-point-num">{idx + 1}</div>
              {renderCheckers(point.color, point.count)}
            </div>
          ))}
        </div>
      </div>

      {isFinished && (
        <button
          className="bg-reset-btn"
          onClick={() => navigate('/battle_lobby', { state: { gameType: 'Backgammon', username } })}
        >
          Play Again
        </button>
      )}
      <button
        className="bg-reset-btn bg-main-btn"
        onClick={() => navigate('/lobby', { state: { username } })}
      >
        Main Lobby
      </button>
    </div>
  );
};

export default Backgammon;
