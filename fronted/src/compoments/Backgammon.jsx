import React, { useState, useEffect, useContext } from 'react';
import { useLocation, useNavigate } from 'react-router-dom';
import { UserContext } from '../UserContext';
import './Backgammon.css';

const POINTS = 24;
const CHECKER_COLORS = { white: '#f7f7f7', black: '#222' };

const Backgammon = () => {
  const { state } = useLocation();
  const { sessionId } = state || {};
  const { username } = useContext(UserContext);
  const navigate = useNavigate();

  const [board, setBoard] = useState(Array.from({ length: POINTS }, () => ({ color: null, count: 0 })));
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
      if (sessionId && username) {
        socket.send(JSON.stringify({ sessionId, username }));
      }
    };

    socket.onmessage = (event) => {
      const state = JSON.parse(event.data);
      setBoard(state.board);
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

  // ====== 2. Player info and perspective ======
  const myIdx = players.indexOf(username);
  const myColor = myIdx === 0 ? "black" : "white";
  const rivalIdx = myIdx === 0 ? 1 : 0;
  const rivalName = players[rivalIdx] || "";
  const rivalColor = myIdx === 0 ? "white" : "black";
  const isWhite = myColor === "white";

  // ====== 3. Board mapping functions ======
  // Map UI index (0-23) to absolute board index
  const getAbsoluteIndex = (uiIdx) => isWhite ? 23 - uiIdx : uiIdx;

  // ====== 4. Move handling ======
  const handlePointClick = (uiIdx) => {
    if (!ws || ws.readyState !== 1 || isFinished || currentPlayer !== username) return;
    const absIdx = getAbsoluteIndex(uiIdx);

    if (selectedFrom === null) {
      const point = board[absIdx];
      if (point.color === myColor && point.count > 0) {
        setSelectedFrom(uiIdx);
      }
      return;
    }

    if (selectedFrom !== null) {
      const absFrom = getAbsoluteIndex(selectedFrom);
      ws.send(JSON.stringify({ username, from: absFrom, to: absIdx }));
      setSelectedFrom(null);
    }
  };

  // ====== 5. Render helpers ======
  const renderCheckers = (color, count) => {
    if (!color || count === 0) return null;
    return (
      <div className={`checkers ${color}`}>
        {Array.from({ length: count }).map((_, i) => (
          <div className="checker" key={i} />
        ))}
      </div>
    );
  };

  // ====== 6. Row indices for rendering ======
  const topRow = Array.from({ length: 12 }, (_, i) => i);      // UI indices 0-11
  const bottomRow = Array.from({ length: 12 }, (_, i) => i + 12); // UI indices 12-23

  // ====== 7. Render ======
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

      {/* Rival name and color above the board */}
      <div style={{ display: 'flex', justifyContent: 'flex-end', width: '100%', marginBottom: 8 }}>
        <div style={{
          fontWeight: 'bold',
          color: rivalColor === 'white' ? CHECKER_COLORS.white : CHECKER_COLORS.black,
          fontSize: 18,
          textShadow: rivalColor === 'white' ? '0 1px 2px #222' : '0 1px 2px #fff'
        }}>
          {rivalName} ({rivalColor})
        </div>
      </div>

      <div className="bg-board">
        <div className="bg-row bg-row-top">
          {topRow.map((uiIdx) => {
            const absIdx = getAbsoluteIndex(uiIdx);
            const point = board[absIdx];
            return (
              <div
                className="bg-point"
                key={uiIdx}
                onClick={() => handlePointClick(uiIdx)}
              >
                <div className="bg-point-num">{uiIdx + 1}</div>
                {renderCheckers(point.color, point.count)}
              </div>
            );
          })}
        </div>
        <div className="bg-bar">
          <div className="bg-dice">
            <span className="bg-die">{dice[0]}</span>
            <span className="bg-die">{dice[1]}</span>
          </div>
        </div>
        <div className="bg-row bg-row-bottom">
          {bottomRow.map((uiIdx) => {
            const absIdx =  getAbsoluteIndex(uiIdx);
            const point = board[absIdx];
            return (
              <div
                className="bg-point"
                key={uiIdx}
                onClick={() => handlePointClick(uiIdx)}
              >
                <div className="bg-point-num">{uiIdx + 1}</div>
                {renderCheckers(point.color, point.count)}
              </div>
            );
          })}
        </div>
      </div>

      {/* User name and color below the board */}
      <div style={{ display: 'flex', justifyContent: 'center', width: '100%', marginTop: 8 }}>
        <div style={{
          fontWeight: 'bold',
          color: myColor === 'white' ? CHECKER_COLORS.white : CHECKER_COLORS.black,
          fontSize: 18,
          textShadow: myColor === 'white' ? '0 1px 2px #222' : '0 1px 2px #fff'
        }}>
          {username} ({myColor})
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
